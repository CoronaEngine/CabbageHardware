#pragma once

#include<Hardware/DeviceManager.h>
#include<Hardware/GlobalContext.h>

class RasterizerPipeline;
class ComputePipeline;
struct HardwareExecutor;

struct CommandRecord
{
    enum class ExecutorType
    {
        Graphics,
        Compute,
        Transfer
    };

    virtual ~CommandRecord() = default;

    virtual void commitCommand(HardwareExecutor &executor) = 0;

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

