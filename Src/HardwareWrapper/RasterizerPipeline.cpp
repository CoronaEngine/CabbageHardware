#include "HardwareWrapperVulkan/PipelineVulkan/RasterizerPipeline.h"

#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

#include <utility>

namespace
{
RasterizerPipelineVulkan *rasterizer_impl(std::uintptr_t id)
{
    if (id == 0)
    {
        return nullptr;
    }

    auto const handle = gRasterizerPipelineStorage.acquire_read(id);
    return handle->impl;
}
} // namespace

RasterizerPipelineBase::RasterizerPipelineBase()
{
    rasterizerPipelineHandle_ = gRasterizerPipelineStorage.allocate_handle();
    auto const handle = gRasterizerPipelineStorage.acquire_write(getRasterizerPipelineID());
    handle->impl = new RasterizerPipelineVulkan();
}

RasterizerPipelineBase::RasterizerPipelineBase(std::string vs,
                                               std::string fs,
                                               uint32_t multiviewCount,
                                               EmbeddedShader::ShaderLanguage vlang,
                                               EmbeddedShader::ShaderLanguage flang,
                                               const std::source_location &src)
{
    rasterizerPipelineHandle_ = gRasterizerPipelineStorage.allocate_handle();
    auto const handle = gRasterizerPipelineStorage.acquire_write(getRasterizerPipelineID());
    handle->impl = new RasterizerPipelineVulkan(std::move(vs), std::move(fs), multiviewCount, vlang, flang, src);
}

RasterizerPipelineBase::RasterizerPipelineBase(const std::vector<uint32_t> &vertexSpirV,
                                               const std::vector<uint32_t> &fragmentSpirV,
                                               uint32_t multiviewCount,
                                               const std::source_location &src)
{
    rasterizerPipelineHandle_ = gRasterizerPipelineStorage.allocate_handle();
    auto const handle = gRasterizerPipelineStorage.acquire_write(getRasterizerPipelineID());
    handle->impl = new RasterizerPipelineVulkan(vertexSpirV, fragmentSpirV, multiviewCount, src);
}

void rasterizerPipelineInitFromCompiler(RasterizerPipelineBase &pipeline,
                                         const EmbeddedShader::ShaderCodeCompiler &vertexCompiler,
                                         const EmbeddedShader::ShaderCodeCompiler &fragmentCompiler,
                                         uint32_t multiviewCount,
                                         const std::source_location &src)
{
    std::lock_guard<std::mutex> lock(pipeline.rasterizerPipelineMutex);
    pipeline.rasterizerPipelineHandle_ = gRasterizerPipelineStorage.allocate_handle();
    auto const handle = gRasterizerPipelineStorage.acquire_write(pipeline.getRasterizerPipelineID());
    handle->impl = new RasterizerPipelineVulkan(vertexCompiler, fragmentCompiler, multiviewCount, src);
}

RasterizerPipelineBase::RasterizerPipelineBase(const RasterizerPipelineBase &other)
{
    std::lock_guard<std::mutex> lock(other.rasterizerPipelineMutex);
    rasterizerPipelineHandle_ = other.rasterizerPipelineHandle_;
    autoBindEntries_ = other.autoBindEntries_;
}

RasterizerPipelineBase::RasterizerPipelineBase(RasterizerPipelineBase &&other) noexcept
{
    std::lock_guard<std::mutex> lock(other.rasterizerPipelineMutex);
    rasterizerPipelineHandle_ = std::move(other.rasterizerPipelineHandle_);
    autoBindEntries_ = std::move(other.autoBindEntries_);
}

RasterizerPipelineBase::~RasterizerPipelineBase() = default;

RasterizerPipelineBase &RasterizerPipelineBase::operator=(const RasterizerPipelineBase &other)
{
    if (this == &other)
    {
        return *this;
    }

    std::scoped_lock lock(rasterizerPipelineMutex, other.rasterizerPipelineMutex);
    rasterizerPipelineHandle_ = other.rasterizerPipelineHandle_;
    autoBindEntries_ = other.autoBindEntries_;
    return *this;
}

RasterizerPipelineBase &RasterizerPipelineBase::operator=(RasterizerPipelineBase &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    std::scoped_lock lock(rasterizerPipelineMutex, other.rasterizerPipelineMutex);
    rasterizerPipelineHandle_ = std::move(other.rasterizerPipelineHandle_);
    autoBindEntries_ = std::move(other.autoBindEntries_);
    return *this;
}

void RasterizerPipelineBase::setDepthImage(HardwareImage &depthImage)
{
    if (auto *impl = rasterizer_impl(getRasterizerPipelineID()))
    {
        impl->setDepthImage(depthImage);
    }
}

void RasterizerPipelineBase::setDepthEnabled(bool enabled)
{
    if (auto *impl = rasterizer_impl(getRasterizerPipelineID()))
    {
        impl->setDepthEnabled(enabled);
    }
}

HardwareImage RasterizerPipelineBase::getDepthImage()
{
    if (auto *impl = rasterizer_impl(getRasterizerPipelineID()))
    {
        return impl->getDepthImage();
    }
    return HardwareImage();
}

void RasterizerPipelineBase::setPushConstantDirect(uint64_t byteOffset, const void *data, size_t size, int32_t bindType)
{
    if (auto *impl = rasterizer_impl(getRasterizerPipelineID()))
    {
        impl->setPushConstantDirect(byteOffset, data, size, bindType);
    }
}

void RasterizerPipelineBase::setResourceDirect(uint64_t byteOffset,
                                               uint32_t typeSize,
                                               const HardwareBuffer &buffer,
                                               int32_t bindType)
{
    if (auto *impl = rasterizer_impl(getRasterizerPipelineID()))
    {
        impl->setResourceDirect(byteOffset, typeSize, buffer, bindType);
    }
}

void RasterizerPipelineBase::setResourceDirect(uint64_t byteOffset,
                                               uint32_t typeSize,
                                               const HardwareImage &image,
                                               int32_t bindType,
                                               uint32_t location)
{
    if (auto *impl = rasterizer_impl(getRasterizerPipelineID()))
    {
        impl->setResourceDirect(byteOffset, typeSize, image, bindType, location);
    }
}

RasterizerPipelineBase &RasterizerPipelineBase::operator()(uint16_t width, uint16_t height)
{
    for (const auto &entry : autoBindEntries_)
    {
        if (void *res = *entry.boundResourceRef)
        {
            setResourceDirect(entry.byteOffset, entry.typeSize, *static_cast<HardwareImage *>(res), entry.bindType, entry.location);
        }
    }

    if (auto *impl = rasterizer_impl(getRasterizerPipelineID()))
    {
        (*impl)(width, height);
    }
    return *this;
}

RasterizerPipelineBase &RasterizerPipelineBase::record(const HardwareBuffer &indexBuffer, const HardwareBuffer &vertexBuffer)
{
    DrawIndexedParams params;
    return record(indexBuffer, vertexBuffer, params);
}

RasterizerPipelineBase &RasterizerPipelineBase::record(const HardwareBuffer &indexBuffer,
                                                       const HardwareBuffer &vertexBuffer,
                                                       const DrawIndexedParams &params)
{
    if (auto *impl = rasterizer_impl(getRasterizerPipelineID()))
    {
        impl->record(indexBuffer, vertexBuffer, params);
    }
    return *this;
}
