#include "CabbageHardware.h"
#include "HardwareCommands.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"
#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"
#include "HardwareWrapperVulkan/PipelineVulkan/RasterizerPipeline.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

#include <memory>
#include <utility>

HardwareExecutor::HardwareExecutor()
{
    executorHandle_ = gExecutorStorage.allocate_handle();
    auto const handle = gExecutorStorage.acquire_write(getExecutorID());
    handle->impl = new HardwareExecutorVulkan();
}

HardwareExecutor::HardwareExecutor(const HardwareExecutor &other)
    : executorHandle_(other.executorHandle_)
{
}

HardwareExecutor::HardwareExecutor(HardwareExecutor &&other) noexcept
    : executorHandle_(std::move(other.executorHandle_))
{
}

HardwareExecutor::~HardwareExecutor() = default;

HardwareExecutor &HardwareExecutor::operator=(const HardwareExecutor &other)
{
    if (this != &other)
    {
        executorHandle_ = other.executorHandle_;
    }
    return *this;
}

HardwareExecutor &HardwareExecutor::operator=(HardwareExecutor &&other) noexcept
{
    if (this != &other)
    {
        executorHandle_ = std::move(other.executorHandle_);
    }
    return *this;
}

HardwareExecutor &HardwareExecutor::operator<<(ComputePipelineBase &computePipeline)
{
    auto const selfId = getExecutorID();
    auto const pipelineId = computePipeline.getComputePipelineID();
    if (selfId == 0 || pipelineId == 0)
    {
        return *this;
    }

    auto const executorHandle = gExecutorStorage.acquire_write(selfId);
    if (!executorHandle->impl)
    {
        return *this;
    }

    auto const pipelineHandle = gComputePipelineStorage.acquire_read(pipelineId);
    if (pipelineHandle->impl)
    {
        *executorHandle->impl << static_cast<CommandRecordVulkan *>(pipelineHandle->impl);

        auto resourceHolder = std::make_shared<ResourceHolderCommand>();
        resourceHolder->computePipelines.push_back(computePipeline);
        executorHandle->impl->pendingResources.push_back(std::move(resourceHolder));
    }

    return *this;
}

HardwareExecutor &HardwareExecutor::operator<<(RasterizerPipelineBase &rasterizerPipeline)
{
    auto const selfId = getExecutorID();
    auto const pipelineId = rasterizerPipeline.getRasterizerPipelineID();
    if (selfId == 0 || pipelineId == 0)
    {
        return *this;
    }

    auto const executorHandle = gExecutorStorage.acquire_write(selfId);
    if (!executorHandle->impl)
    {
        return *this;
    }

    auto const pipelineHandle = gRasterizerPipelineStorage.acquire_read(pipelineId);
    if (pipelineHandle->impl)
    {
        *executorHandle->impl << static_cast<CommandRecordVulkan *>(pipelineHandle->impl);

        auto resourceHolder = std::make_shared<ResourceHolderCommand>();
        resourceHolder->rasterizerPipelines.push_back(rasterizerPipeline);
        executorHandle->impl->pendingResources.push_back(std::move(resourceHolder));
    }

    return *this;
}

HardwareExecutor &HardwareExecutor::operator<<(HardwareExecutor &other)
{
    return other;
}

HardwareExecutor &HardwareExecutor::operator<<(const CopyCommand &cmd)
{
    if (!cmd.impl)
    {
        return *this;
    }

    auto const selfId = getExecutorID();
    if (selfId == 0)
    {
        return *this;
    }

    auto executorHandle = gExecutorStorage.acquire_write(selfId);
    if (!executorHandle->impl)
    {
        return *this;
    }

    if (CommandRecordVulkan *record = cmd.impl->getCommandRecord())
    {
        *executorHandle->impl << record;
        executorHandle->impl->pendingResources.push_back(cmd.impl);
    }

    return *this;
}

HardwareExecutor &HardwareExecutor::wait(HardwareExecutor &other)
{
    auto const selfId = getExecutorID();
    auto const otherId = other.getExecutorID();
    if (selfId == 0 || otherId == 0 || selfId == otherId)
    {
        return *this;
    }

    if (selfId < otherId)
    {
        auto const selfHandle = gExecutorStorage.acquire_write(selfId);
        auto const otherHandle = gExecutorStorage.acquire_read(otherId);
        if (selfHandle->impl && otherHandle->impl)
        {
            selfHandle->impl->wait(*otherHandle->impl);
        }
    }
    else
    {
        auto const otherHandle = gExecutorStorage.acquire_read(otherId);
        auto const selfHandle = gExecutorStorage.acquire_write(selfId);
        if (selfHandle->impl && otherHandle->impl)
        {
            selfHandle->impl->wait(*otherHandle->impl);
        }
    }

    return *this;
}

HardwareExecutor &HardwareExecutor::commit()
{
    auto const selfId = getExecutorID();
    if (selfId == 0)
    {
        return *this;
    }

    auto handle = gExecutorStorage.acquire_write(selfId);
    if (handle->impl)
    {
        handle->impl->commit();
    }
    return *this;
}

void HardwareExecutor::waitForDeferredResources()
{
    auto const selfId = getExecutorID();
    if (selfId == 0)
    {
        return;
    }

    auto handle = gExecutorStorage.acquire_write(selfId);
    if (handle->impl)
    {
        handle->impl->waitForAllDeferredResources();
    }
}

void HardwareExecutor::cleanupDeferredResources()
{
    auto const selfId = getExecutorID();
    if (selfId == 0)
    {
        return;
    }

    auto handle = gExecutorStorage.acquire_write(selfId);
    if (handle->impl)
    {
        handle->impl->cleanupCompletedResources();
    }
}

namespace
{
struct ImageTransitionCommandImpl : CopyCommandImpl
{
    HardwareImage image;
    ResourceState state{ResourceState::Unknown};
    TextureSubresourceSet subresources{};
    std::unique_ptr<TransitionImageStateCommand> command;

    ImageTransitionCommandImpl(const HardwareImage &imageValue,
                               ResourceState stateValue,
                               TextureSubresourceSet subresourcesValue)
        : image(imageValue), state(stateValue), subresources(subresourcesValue)
    {
    }

    CommandRecordVulkan *getCommandRecord() override
    {
        if (image.getImageID() == 0)
        {
            return nullptr;
        }

        auto handle = globalImageStorages.acquire_write(image.getImageID());
        command = std::make_unique<TransitionImageStateCommand>(*handle, state, subresources);
        return command.get();
    }
};

struct BufferTransitionCommandImpl : CopyCommandImpl
{
    HardwareBuffer buffer;
    ResourceState state{ResourceState::Unknown};
    std::unique_ptr<TransitionBufferStateCommand> command;

    BufferTransitionCommandImpl(const HardwareBuffer &bufferValue, ResourceState stateValue)
        : buffer(bufferValue), state(stateValue)
    {
    }

    CommandRecordVulkan *getCommandRecord() override
    {
        if (buffer.getBufferID() == 0)
        {
            return nullptr;
        }

        auto handle = globalBufferStorages.acquire_write(buffer.getBufferID());
        command = std::make_unique<TransitionBufferStateCommand>(*handle, state);
        return command.get();
    }
};
} // namespace

HardwareExecutor &HardwareExecutor::setAutomaticBarriers(bool enabled)
{
    auto const selfId = getExecutorID();
    if (selfId == 0)
    {
        return *this;
    }

    auto handle = gExecutorStorage.acquire_write(selfId);
    if (handle->impl)
    {
        handle->impl->setAutomaticBarriers(enabled);
    }
    return *this;
}

HardwareExecutor &HardwareExecutor::transition(HardwareImage &image,
                                               ResourceState state,
                                               TextureSubresourceSet subresources)
{
    auto const selfId = getExecutorID();
    if (selfId == 0)
    {
        return *this;
    }

    auto impl = std::make_shared<ImageTransitionCommandImpl>(image, state, subresources);
    auto handle = gExecutorStorage.acquire_write(selfId);
    if (handle->impl)
    {
        if (CommandRecordVulkan *record = impl->getCommandRecord())
        {
            *handle->impl << record;
            handle->impl->pendingResources.push_back(std::move(impl));
        }
    }
    return *this;
}

HardwareExecutor &HardwareExecutor::transition(HardwareBuffer &buffer, ResourceState state)
{
    auto const selfId = getExecutorID();
    if (selfId == 0)
    {
        return *this;
    }

    auto impl = std::make_shared<BufferTransitionCommandImpl>(buffer, state);
    auto handle = gExecutorStorage.acquire_write(selfId);
    if (handle->impl)
    {
        if (CommandRecordVulkan *record = impl->getCommandRecord())
        {
            *handle->impl << record;
            handle->impl->pendingResources.push_back(std::move(impl));
        }
    }
    return *this;
}
