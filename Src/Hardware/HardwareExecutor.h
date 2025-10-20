#pragma once

#include<Hardware/DeviceManager.h>
#include<Hardware/GlobalContext.h>

class RasterizerPipeline;
class ComputePipeline;

struct CommandRecord
{
    enum class ExecutorType
    {
        Graphics,
        Compute,
        Transfer
    };

    virtual void commitCommand(const VkCommandBuffer& commandBuffer)
    {
    }

    ExecutorType executorType;
};

struct CopyBufferCommand : public CommandRecord
{
    VkBuffer srcBuffer;
    VkBuffer dstBuffer;
    std::vector<VkBufferCopy> regions;

    CopyBufferCommand(VkBuffer src, VkBuffer dst, const std::vector<VkBufferCopy> &copyRegions)
        : srcBuffer(src), dstBuffer(dst), regions(copyRegions)
    {
        executorType = ExecutorType::Transfer;
    }

    void commitCommand(const VkCommandBuffer &commandBuffer) override
    {
        vkCmdCopyBuffer(
            commandBuffer,
            srcBuffer,
            dstBuffer,
            static_cast<uint32_t>(regions.size()),
            regions.data());
    }
};

struct CopyImageCommand : public CommandRecord
{
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    std::vector<VkImageCopy> regions;

    CopyImageCommand(
        VkImage srcImg, VkImageLayout srcLayout,
        VkImage dstImg, VkImageLayout dstLayout,
        const std::vector<VkImageCopy> &copyRegions)
        : srcImage(srcImg), srcImageLayout(srcLayout),
          dstImage(dstImg), dstImageLayout(dstLayout),
          regions(copyRegions)
    {
        executorType = ExecutorType::Transfer;
    }

    void commitCommand(const VkCommandBuffer &commandBuffer) override
    {
        vkCmdCopyImage(
            commandBuffer,
            srcImage,
            srcImageLayout,
            dstImage,
            dstImageLayout,
            static_cast<uint32_t>(regions.size()),
            regions.data());
    }
};

struct HardwareExecutor
{
    HardwareExecutor(std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext = globalHardwareContext.mainDevice)
        : hardwareContext(hardwareContext)
    {
    }

    ~HardwareExecutor() = default;

    HardwareExecutor &operator<<(const HardwareExecutor &other)
    {
        return *this;
    }

    HardwareExecutor &operator()(CommandRecord::ExecutorType type = CommandRecord::ExecutorType::Graphics, HardwareExecutor *waitExecutor = nullptr);

    HardwareExecutor &operator<<(std::function<void(const VkCommandBuffer &commandBuffer)> commandsFunction)
    {
        commandsFunction(currentRecordQueue->commandBuffer);
        return *this;
    }

    HardwareExecutor &commit(std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                             std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                             VkFence fence = VK_NULL_HANDLE);


  private:
    friend HardwareExecutor &operator<<(HardwareExecutor &executor, RasterizerPipeline &other);
    friend HardwareExecutor &operator<<(HardwareExecutor &executor, ComputePipeline &other);

    bool recordingBegan = false;
    bool computePipelineBegin = false;
    bool rasterizerPipelineBegin = false;

    CommandRecord::ExecutorType queueType = CommandRecord::ExecutorType::Graphics;
    DeviceManager::QueueUtils *currentRecordQueue = nullptr;

    std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext;

    std::vector<CommandRecord> commandList;
};