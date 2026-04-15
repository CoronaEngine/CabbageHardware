#include "ResourcePool.h"

BufferResourceRegistry globalBufferStorages{"HardwareBuffer"};
ImageResourceRegistry globalImageStorages{"HardwareImage"};
SamplerResourceRegistry globalSamplerStorages{"HardwareSampler"};
RasterizerPipelineRegistry gRasterizerPipelineStorage{"RasterizerPipeline"};
DisplayerResourceRegistry globalDisplayerStorages{"HardwareDisplayer"};
ComputePipelineRegistry gComputePipelineStorage{"ComputePipeline"};
ExecutorResourceRegistry gExecutorStorage{"HardwareExecutor"};
PushConstantResourceRegistry globalPushConstantStorages{"HardwarePushConstant"};
