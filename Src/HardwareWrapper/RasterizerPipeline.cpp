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

RasterizerPipeline::RasterizerPipeline() {
    auto id = gRasterizerPipelineStorage.allocate();
    auto handle = gRasterizerPipelineStorage.acquire_write(id);
    handle->impl = new RasterizerPipelineVulkan();
    handle->refCount = 1;
    rasterizerPipelineID = std::make_shared<uintptr_t>(id);
}

RasterizerPipeline::RasterizerPipeline(std::string vs, std::string fs, uint32_t multiviewCount, EmbeddedShader::ShaderLanguage vlang, EmbeddedShader::ShaderLanguage flang, const std::source_location& src) {
    auto id = gRasterizerPipelineStorage.allocate();
    auto handle = gRasterizerPipelineStorage.acquire_write(id);
    handle->impl = new RasterizerPipelineVulkan(vs, fs, multiviewCount, vlang, flang, src);
    handle->refCount = 1;
    rasterizerPipelineID = std::make_shared<uintptr_t>(id);
}

RasterizerPipeline::RasterizerPipeline(const RasterizerPipeline& other)
    : rasterizerPipelineID(other.rasterizerPipelineID) {
    if (*rasterizerPipelineID > 0) {
        auto const handle = gRasterizerPipelineStorage.acquire_write(*rasterizerPipelineID);
        incRaster(handle);
    }
}

RasterizerPipeline::~RasterizerPipeline() {
    if (rasterizerPipelineID && *rasterizerPipelineID > 0) {
        bool destroy = false;
        if (auto const handle = gRasterizerPipelineStorage.acquire_write(*rasterizerPipelineID); decRaster(handle)) {
            destroy = true;
        }
        if (destroy) {
            gRasterizerPipelineStorage.deallocate(*rasterizerPipelineID);
        }
    }
}

RasterizerPipeline& RasterizerPipeline::operator=(const RasterizerPipeline& other) {
    if (this != &other) {
        {
            auto const handle = gRasterizerPipelineStorage.acquire_write(*other.rasterizerPipelineID);
            incRaster(handle);
        }
        {
            if (rasterizerPipelineID && *rasterizerPipelineID > 0) {
                bool destroy = false;
                if (auto const handle = gRasterizerPipelineStorage.acquire_write(*rasterizerPipelineID); decRaster(handle)) {
                    destroy = true;
                }
                if (destroy) {
                    gRasterizerPipelineStorage.deallocate(*rasterizerPipelineID);
                }
            }
        }
        *rasterizerPipelineID = *other.rasterizerPipelineID;
    }
    return *this;
}

void RasterizerPipeline::setDepthImage(HardwareImage& depthImage) {
    auto handle = gRasterizerPipelineStorage.acquire_read(*rasterizerPipelineID);
    handle->impl->setDepthImage(depthImage);
}

HardwareImage RasterizerPipeline::getDepthImage() {
    HardwareImage img;
    auto handle = gRasterizerPipelineStorage.acquire_read(*rasterizerPipelineID);
    img = handle->impl->getDepthImage();
    return img;
}

std::variant<HardwarePushConstant, HardwareBuffer, HardwareImage> RasterizerPipeline::operator[](const std::string& resourceName) {
    std::variant<HardwarePushConstant, HardwareBuffer, HardwareImage> r;
    auto handle = gRasterizerPipelineStorage.acquire_read(*rasterizerPipelineID);
    r = (*handle->impl)[resourceName];
    return r;
}

RasterizerPipeline& RasterizerPipeline::operator()(uint16_t width, uint16_t height) {
    auto handle = gRasterizerPipelineStorage.acquire_read(*rasterizerPipelineID);
    (*handle->impl)(width, height);
    return *this;
}

RasterizerPipeline& RasterizerPipeline::record(const HardwareBuffer& indexBuffer, const HardwareBuffer& vertexBuffer) {
    auto handle = gRasterizerPipelineStorage.acquire_read(*rasterizerPipelineID);
    handle->impl->record(indexBuffer, vertexBuffer);
    return *this;
}