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

    virtual ~CommandRecord() = default;

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

struct CopyBufferToImageCommand : public CommandRecord
{
    VkBuffer srcBuffer;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    std::vector<VkBufferImageCopy> regions; // 使用 VkBufferImageCopy

    CopyBufferToImageCommand(
        VkBuffer srcBuf,
        VkImage dstImg,
        VkImageLayout dstLayout,
        const std::vector<VkBufferImageCopy> &copyRegions)
        : srcBuffer(srcBuf),
          dstImage(dstImg),
          dstImageLayout(dstLayout),
          regions(copyRegions)
    {
        // 拷贝操作通常在 Transfer 队列中执行
        executorType = ExecutorType::Transfer;
    }

    void commitCommand(const VkCommandBuffer &commandBuffer) override
    {
        vkCmdCopyBufferToImage(
            commandBuffer,
            srcBuffer,
            dstImage,
            dstImageLayout,
            static_cast<uint32_t>(regions.size()),
            regions.data());
    }
};

struct CopyImageToBufferCommand : public CommandRecord
{
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkBuffer dstBuffer;
    std::vector<VkBufferImageCopy> regions; // 同样使用 VkBufferImageCopy

    CopyImageToBufferCommand(
        VkImage srcImg,
        VkImageLayout srcLayout,
        VkBuffer dstBuf,
        const std::vector<VkBufferImageCopy> &copyRegions)
        : srcImage(srcImg),
          srcImageLayout(srcLayout),
          dstBuffer(dstBuf),
          regions(copyRegions)
    {
        // 拷贝操作通常在 Transfer 队列中执行
        executorType = ExecutorType::Transfer;
    }

    void commitCommand(const VkCommandBuffer &commandBuffer) override
    {
        vkCmdCopyImageToBuffer(
            commandBuffer,
            srcImage,
            srcImageLayout,
            dstBuffer,
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

    std::vector<std::shared_ptr<CommandRecord>> commandList;
};