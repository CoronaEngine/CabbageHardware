#include "HardwareWrapperVulkan/PipelineVulkan/RasterizerPipeline.h"

#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

static void incRaster(const Corona::Kernel::Utils::Storage<RasterizerPipelineWrap>::WriteHandle& handle) {
    ++handle->refCount;
}

static bool decRaster(const Corona::Kernel::Utils::Storage<RasterizerPipelineWrap>::WriteHandle& handle) {
    if (--handle->refCount == 0) {
        delete handle->impl;
        handle->impl = nullptr;
        return true;
    }
    return false;
}

RasterizerPipeline::RasterizerPipeline()
    : rasterizerPipelineID(gRasterizerPipelineStorage.allocate()) {
    auto handle = gRasterizerPipelineStorage.acquire_write(rasterizerPipelineID.load(std::memory_order_acquire));
    handle->impl = new RasterizerPipelineVulkan();
}

RasterizerPipeline::RasterizerPipeline(std::string vs, std::string fs, uint32_t multiviewCount, EmbeddedShader::ShaderLanguage vlang, EmbeddedShader::ShaderLanguage flang, const std::source_location& src)
    : rasterizerPipelineID(gRasterizerPipelineStorage.allocate()) {
    auto handle = gRasterizerPipelineStorage.acquire_write(rasterizerPipelineID.load(std::memory_order_acquire));
    handle->impl = new RasterizerPipelineVulkan(vs, fs, multiviewCount, vlang, flang, src);
}

RasterizerPipeline::RasterizerPipeline(const RasterizerPipeline& other)
    : rasterizerPipelineID(other.rasterizerPipelineID.load(std::memory_order_acquire)) {
    if (rasterizerPipelineID.load(std::memory_order_acquire) > 0) {
        auto const handle = gRasterizerPipelineStorage.acquire_write(rasterizerPipelineID.load(std::memory_order_acquire));
        incRaster(handle);
    }
}

RasterizerPipeline::RasterizerPipeline(RasterizerPipeline&& other) noexcept
    : rasterizerPipelineID(other.rasterizerPipelineID.load(std::memory_order_acquire)) {
    other.rasterizerPipelineID.store(0, std::memory_order_release);
}

RasterizerPipeline::~RasterizerPipeline() {
    if (auto const self_id = rasterizerPipelineID.load(std::memory_order_acquire);
        self_id > 0) {
        bool should_destroy_self = false;
        if (auto const handle = gRasterizerPipelineStorage.acquire_write(self_id); decRaster(handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gRasterizerPipelineStorage.deallocate(self_id);
        }
        rasterizerPipelineID.store(0, std::memory_order_release);
    }
}

RasterizerPipeline& RasterizerPipeline::operator=(const RasterizerPipeline& other) {
    if (this == &other) {
        return *this;
    }
    auto const self_id = rasterizerPipelineID.load(std::memory_order_acquire);
    auto const other_id = other.rasterizerPipelineID.load(std::memory_order_acquire);
    if (self_id == 0 && other_id == 0) {
        // 都未初始化，直接返回
        return *this;
    }
    if (self_id == other_id) {
        // 已经指向同一个资源，无需操作
        return *this;
    }
    bool should_destroy_self = false;
    if (other_id == 0) {
        // 释放自身资源
        if (auto const self_handle = gRasterizerPipelineStorage.acquire_write(self_id);
            decRaster(self_handle)) {
            gRasterizerPipelineStorage.deallocate(self_id);
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gRasterizerPipelineStorage.deallocate(self_id);
        }
        rasterizerPipelineID.store(0, std::memory_order_release);
    }
    if (self_id == 0) {
        // 直接拷贝
        if (auto const other_handle = gRasterizerPipelineStorage.acquire_write(other_id);
            other_handle->impl) {
            incRaster(other_handle);
        }
        rasterizerPipelineID.store(other_id, std::memory_order_release);
    }
    if (self_id < other_id) {
        auto const self_handle = gRasterizerPipelineStorage.acquire_write(self_id);
        auto const other_handle = gRasterizerPipelineStorage.acquire_write(other_id);
        if (other_handle->impl) {
            incRaster(other_handle);
        }
        if (decRaster(self_handle)) {
            should_destroy_self = true;
        }
    } else {
        auto const other_handle = gRasterizerPipelineStorage.acquire_write(other_id);
        auto const self_handle = gRasterizerPipelineStorage.acquire_write(self_id);
        if (other_handle->impl) {
            incRaster(other_handle);
        }
        if (decRaster(self_handle)) {
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
    bool should_destroy_self = false;
    if (auto const self_id = rasterizerPipelineID.load(std::memory_order_acquire);
        self_id > 0) {
        if (auto const self_handle = gRasterizerPipelineStorage.acquire_write(self_id);
            decRaster(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gRasterizerPipelineStorage.deallocate(self_id);
        }
    }
    rasterizerPipelineID.store(other.rasterizerPipelineID.load(std::memory_order_acquire), std::memory_order_release);
    other.rasterizerPipelineID.store(0, std::memory_order_release);
    return *this;
}

void RasterizerPipeline::setDepthImage(HardwareImage& depthImage) {
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    handle->impl->setDepthImage(depthImage);
}

HardwareImage RasterizerPipeline::getDepthImage() {
    HardwareImage img;
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    img = handle->impl->getDepthImage();
    return img;
}

ResourceProxy RasterizerPipeline::operator[](const std::string& resourceName) {
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    return (*handle->impl)[resourceName];
}

RasterizerPipeline& RasterizerPipeline::operator()(uint16_t width, uint16_t height) {
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    (*handle->impl)(width, height);
    return *this;
}

RasterizerPipeline& RasterizerPipeline::record(const HardwareBuffer& indexBuffer, const HardwareBuffer& vertexBuffer) {
    auto handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipelineID.load(std::memory_order_acquire));
    handle->impl->record(indexBuffer, vertexBuffer);
    return *this;
}