#pragma once

#include<Hardware/DeviceManager.h>
#include<Hardware/GlobalContext.h>

class RasterizerPipeline;
class ComputePipeline;

struct AbstractCommand
{
    enum class ExecutorType
    {
        Graphics,
        Compute,
        Transfer
    };

    virtual ~AbstractCommand() = default;

    virtual void Replay(const VkCommandBuffer &commandBuffer) const = 0;

    ExecutorType executorType;
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

    HardwareExecutor &operator()(AbstractCommand::ExecutorType type = AbstractCommand::ExecutorType::Graphics, HardwareExecutor *waitExecutor = nullptr);

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

    AbstractCommand::ExecutorType queueType = AbstractCommand::ExecutorType::Graphics;
    DeviceManager::QueueUtils *currentRecordQueue = nullptr;

    std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext;
};