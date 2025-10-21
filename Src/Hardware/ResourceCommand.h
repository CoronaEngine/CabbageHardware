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

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.copyBuffer(hardwareExecutor.currentRecordQueue->commandBuffer, srcBuffer, dstBuffer);
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

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.copyImage(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstImage);
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

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.copyBufferToImage(hardwareExecutor.currentRecordQueue->commandBuffer, srcBuffer, dstImage);
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

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.copyImageToBuffer(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstBuffer);
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

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Graphics;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.blitImage(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstImage);
    }
};
