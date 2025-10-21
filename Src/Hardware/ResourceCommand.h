#include "HardwareExecutor.h"



struct CopyBufferCommand : public CommandRecord
{
    ResourceManager::BufferHardwareWrap &srcBuffer;
    ResourceManager::BufferHardwareWrap &dstBuffer;

    CopyBufferCommand(ResourceManager::BufferHardwareWrap &src, ResourceManager::BufferHardwareWrap &dst)
        : srcBuffer(src), dstBuffer(dst)
    {
        executorType = ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.copyBuffer(hardwareExecutor.currentRecordQueue->commandBuffer,srcBuffer, dstBuffer);
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

    void commitCommand(VkCommandBuffer &commandBuffer) override
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

    void commitCommand(VkCommandBuffer &commandBuffer) override
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

    void commitCommand(VkCommandBuffer &commandBuffer) override
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

    void commitCommand(VkCommandBuffer &commandBuffer) override
    {
    }
};
