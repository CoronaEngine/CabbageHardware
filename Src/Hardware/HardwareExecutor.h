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

    virtual void commitCommand(const VkCommandBuffer &commandBuffer) = 0;

    ExecutorType executorType;
};

struct CopyBufferCommand : public CommandRecord
{
    ResourceManager::BufferHardwareWrap &srcBuffer;
    ResourceManager::BufferHardwareWrap &dstBuffer;

    CopyBufferCommand(ResourceManager::BufferHardwareWrap &src, ResourceManager::BufferHardwareWrap &dst)
        : srcBuffer(src), dstBuffer(dst)
    {
        executorType = ExecutorType::Transfer;
    }

    void commitCommand(const VkCommandBuffer &commandBuffer) override
    {
    }
};

struct CopyImageCommand : public CommandRecord
{
    ResourceManager::ImageHardwareWrap &srcImage;
    ResourceManager::ImageHardwareWrap &dstImage;

    CopyImageCommand(ResourceManager::ImageHardwareWrap &srcImg, ResourceManager::ImageHardwareWrap &dstImg)
        : srcImage(srcImg), dstImage(dstImg)
    {
        executorType = ExecutorType::Transfer;
    }

    void commitCommand(const VkCommandBuffer &commandBuffer) override
    {
    }
};

struct CopyBufferToImageCommand : public CommandRecord
{
    ResourceManager::BufferHardwareWrap &srcBuffer;
    ResourceManager::ImageHardwareWrap &dstImage;

    CopyBufferToImageCommand(ResourceManager::BufferHardwareWrap &srcBuf, ResourceManager::ImageHardwareWrap &dstImg)
        : srcBuffer(srcBuf), dstImage(dstImg)
    {
        executorType = ExecutorType::Transfer;
    }

    void commitCommand(const VkCommandBuffer &commandBuffer) override
    {
    }
};

struct CopyImageToBufferCommand : public CommandRecord
{
    ResourceManager::ImageHardwareWrap &srcImage;
    ResourceManager::BufferHardwareWrap &dstBuffer;

    CopyImageToBufferCommand(ResourceManager::ImageHardwareWrap &srcImg, ResourceManager::BufferHardwareWrap &dstBuf)
        : srcImage(srcImg), dstBuffer(dstBuf)
    {
        executorType = ExecutorType::Transfer;
    }

    void commitCommand(const VkCommandBuffer &commandBuffer) override
    {
    }
};

struct BlitImageCommand : public CommandRecord
{
    ResourceManager::ImageHardwareWrap &srcImage;
    ResourceManager::ImageHardwareWrap &dstImage;

    BlitImageCommand(ResourceManager::ImageHardwareWrap &srcImg, ResourceManager::ImageHardwareWrap &dstImg)
        : srcImage(srcImg), dstImage(dstImg)
    {
        executorType = ExecutorType::Graphics;
    }

    void commitCommand(const VkCommandBuffer &commandBuffer) override
    {
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