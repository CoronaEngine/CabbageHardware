#pragma once

#include<Hardware/DeviceManager.h>
#include<Hardware/GlobalContext.h>

class RasterizerPipeline;
class ComputePipeline;

struct HardwareExecutor
{
    enum class ExecutorType
    {
        Graphics,
        Compute,
        Transfer
    };

    HardwareExecutor(std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext = globalHardwareContext.mainDevice)
    {
    }

    ~HardwareExecutor() = default;

    HardwareExecutor &operator<<(const HardwareExecutor &other)
    {
        return *this;
    }

    HardwareExecutor &operator()(ExecutorType type, HardwareExecutor *waitExecutor = nullptr);

    HardwareExecutor &commit();

  private:
    friend HardwareExecutor &operator<<(HardwareExecutor &executor, RasterizerPipeline &other);
    friend HardwareExecutor &operator<<(HardwareExecutor &executor, ComputePipeline &other);

    bool computePipelineBegin = false;
    bool rasterizerPipelineBegin = false;

    ExecutorType type;
};
