#include "HardwareWrapperVulkan/PipelineVulkan/RasterizerPipeline.h"

#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

static void incRaster(uint32_t id, const Corona::Kernel::Utils::Storage<RasterizerPipelineWrap>::WriteHandle& handle) {
    ++handle->refCount;
    CFW_LOG_TRACE("RasterizerPipeline ref++: id={}, count={}", id, handle->refCount);
}

static bool decRaster(uint32_t id, const Corona::Kernel::Utils::Storage<RasterizerPipelineWrap>::WriteHandle& handle) {
    int count = --handle->refCount;
    CFW_LOG_TRACE("RasterizerPipeline ref--: id={}, count={}", id, count);
    if (count == 0) {
        delete handle->impl;
        handle->impl = nullptr;
        CFW_LOG_TRACE("RasterizerPipeline destroyed: id={}", id);
        return true;
    }
    return false;
}

RasterizerPipeline::RasterizerPipeline() {
    auto id = gRasterizerPipelineStorage.allocate();
    rasterizerPipelineID.store(id, std::memory_order_release);
    auto handle = gRasterizerPipelineStorage.acquire_write(id);
    handle->impl = new RasterizerPipelineVulkan();
    CFW_LOG_TRACE("RasterizerPipeline created: id={}", id);
}

RasterizerPipeline::RasterizerPipeline(std::string vs, std::string fs, uint32_t multiviewCount, EmbeddedShader::ShaderLanguage vlang, EmbeddedShader::ShaderLanguage flang, const std::source_location& src) {
    auto id = gRasterizerPipelineStorage.allocate();
    rasterizerPipelineID.store(id, std::memory_order_release);
    auto handle = gRasterizerPipelineStorage.acquire_write(id);
    handle->impl = new RasterizerPipelineVulkan(vs, fs, multiviewCount, vlang, flang, src);
    CFW_LOG_TRACE("RasterizerPipeline created: id={}", id);
}

RasterizerPipeline::RasterizerPipeline(const RasterizerPipeline& other) {
    std::lock_guard<std::mutex> lock(other.rasterizerPipelineMutex);
    auto const other_id = other.rasterizerPipelineID.load(std::memory_order_acquire);
    rasterizerPipelineID.store(other_id, std::memory_order_release);
    if (other_id > 0) {
        auto const handle = gRasterizerPipelineStorage.acquire_write(other_id);
        incRaster(other_id, handle);
    }
}

RasterizerPipeline::RasterizerPipeline(RasterizerPipeline&& other) noexcept {
    std::lock_guard<std::mutex> lock(other.rasterizerPipelineMutex);
    auto const other_id = other.rasterizerPipelineID.load(std::memory_order_acquire);
    rasterizerPipelineID.store(other_id, std::memory_order_release);
    other.rasterizerPipelineID.store(0, std::memory_order_release);
}

RasterizerPipeline::~RasterizerPipeline() {
    auto const self_id = rasterizerPipelineID.load(std::memory_order_acquire);
    if (self_id > 0) {
        bool destroy = false;
        if (auto const handle = gRasterizerPipelineStorage.acquire_write(self_id); decRaster(self_id, handle)) {
            destroy = true;
        }
        if (destroy) {
            gRasterizerPipelineStorage.deallocate(self_id);
        }
        rasterizerPipelineID.store(0, std::memory_order_release);
    }
}

RasterizerPipeline& RasterizerPipeline::operator=(const RasterizerPipeline& other) {
    if (this == &other) {
        return *this;
    }
    std::scoped_lock lock(rasterizerPipelineMutex, other.rasterizerPipelineMutex);
    auto const self_id = rasterizerPipelineID.load(std::memory_order_acquire);
    auto const other_id = other.rasterizerPipelineID.load(std::memory_order_acquire);

    if (self_id == 0 && other_id == 0) {
        return *this;
    }
    if (self_id == other_id) {
        return *this;
    }

    if (other_id == 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = gRasterizerPipelineStorage.acquire_write(self_id);
            decRaster(self_id, self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gRasterizerPipelineStorage.deallocate(self_id);
        }
        rasterizerPipelineID.store(0, std::memory_order_release);
        return *this;
    }

    if (self_id == 0) {
        rasterizerPipelineID.store(other_id, std::memory_order_release);
        auto const other_handle = gRasterizerPipelineStorage.acquire_write(other_id);
        incRaster(other_id, other_handle);
        return *this;
    }

    bool should_destroy_self = false;
    if (self_id < other_id) {
        auto const self_handle = gRasterizerPipelineStorage.acquire_write(self_id);
        auto const other_handle = gRasterizerPipelineStorage.acquire_write(other_id);
        incRaster(other_id, other_handle);
        if (decRaster(self_id, self_handle)) {
            should_destroy_self = true;
        }
    } else {
        auto const other_handle = gRasterizerPipelineStorage.acquire_write(other_id);
        auto const self_handle = gRasterizerPipelineStorage.acquire_write(self_id);
        incRaster(other_id, other_handle);
        if (decRaster(self_id, self_handle)) {
            should_destroy_self = true;
        }
    }
    if (should_destroy_self) {
        gRasterizerPipelineStorage.deallocate(self_id);
    }
    rasterizerPipelineID.store(other_id, std::memory_order_release);
    return *this;
}

RasterizerPipeline& RasterizerPipeline::operator=(RasterizerPipeline&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    std::scoped_lock lock(rasterizerPipelineMutex, other.rasterizerPipelineMutex);
    auto const self_id = rasterizerPipelineID.load(std::memory_order_acquire);
    auto const other_id = other.rasterizerPipelineID.load(std::memory_order_acquire);

    if (self_id > 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = gRasterizerPipelineStorage.acquire_write(self_id);
            decRaster(self_id, self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gRasterizerPipelineStorage.deallocate(self_id);
        }
    }
    rasterizerPipelineID.store(other_id, std::memory_order_release);
    other.rasterizerPipelineID.store(0, std::memory_order_release);
    return *this;
}

void RasterizerPipeline::setDepthImage(HardwareImage& depthImage) {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    handle->impl->setDepthImage(depthImage);
}

HardwareImage RasterizerPipeline::getDepthImage() {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    HardwareImage img;
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    img = handle->impl->getDepthImage();
    return img;
}

ResourceProxy RasterizerPipeline::operator[](const std::string& resourceName) {
    return ResourceProxy(this, resourceName);
}

void RasterizerPipeline::setPushConstant(const std::string& name, const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    handle->impl->setPushConstant(name, data, size);
}

void RasterizerPipeline::setResource(const std::string& name, const HardwareBuffer& buffer) {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    handle->impl->setResource(name, buffer);
}

void RasterizerPipeline::setResource(const std::string& name, const HardwareImage& image) {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    handle->impl->setResource(name, image);
}

HardwarePushConstant RasterizerPipeline::getPushConstant(const std::string& name) const {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    return handle->impl->getPushConstant(name);
}

HardwareBuffer RasterizerPipeline::getBuffer(const std::string& name) const {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    return handle->impl->getBuffer(name);
}

HardwareImage RasterizerPipeline::getImage(const std::string& name) const {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    return handle->impl->getImage(name);
}

RasterizerPipeline& RasterizerPipeline::operator()(uint16_t width, uint16_t height) {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    (*handle->impl)(width, height);
    return *this;
}

RasterizerPipeline& RasterizerPipeline::record(const HardwareBuffer& indexBuffer, const HardwareBuffer& vertexBuffer) {
    std::lock_guard<std::mutex> lock(rasterizerPipelineMutex);
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    handle->impl->record(indexBuffer, vertexBuffer);
    return *this;
}