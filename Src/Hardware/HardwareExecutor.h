#pragma once

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

    HardwareExecutor() = default;
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
