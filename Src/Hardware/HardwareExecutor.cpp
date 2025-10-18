#include"HardwareExecutor.h"

#include <Hardware/GlobalContext.h>



HardwareExecutor &HardwareExecutor::operator()(ExecutorType type, HardwareExecutor *waitExecutor)
{
    switch (type)
    {
    case HardwareExecutor::ExecutorType::Graphics:
        globalHardwareContext.mainDevice->deviceManager.startCommands(DeviceManager::QueueType::GraphicsQueue);
        break;
    case HardwareExecutor::ExecutorType::Compute:
        globalHardwareContext.mainDevice->deviceManager.startCommands(DeviceManager::QueueType::ComputeQueue);
        break;
    case HardwareExecutor::ExecutorType::Transfer:
        globalHardwareContext.mainDevice->deviceManager.startCommands(DeviceManager::QueueType::TransferQueue);
        break;
    default:
        globalHardwareContext.mainDevice->deviceManager.startCommands(DeviceManager::QueueType::GraphicsQueue);
        break;
    }

    this->type = type;
    return *this;
}


HardwareExecutor& HardwareExecutor::commit()
{
    if (rasterizerPipelineBegin)
    {
        auto runCommand = [&](const VkCommandBuffer &commandBuffer) {
            vkCmdEndRenderPass(commandBuffer);
        };

        globalHardwareContext.mainDevice->deviceManager << runCommand;
    }

    if (computePipelineBegin)
    {
    }

    if (rasterizerPipelineBegin || computePipelineBegin)
    {
        globalHardwareContext.mainDevice->deviceManager << globalHardwareContext.mainDevice->deviceManager.endCommands();

        computePipelineBegin = false;
        rasterizerPipelineBegin = false;
    }

    return *this;
}