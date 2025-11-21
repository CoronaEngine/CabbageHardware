#include "HardwareWrapperVulkan/PipelineVulkan/RasterizerPipeline.h"

#include "CabbageHardware.h"

HardwareExecutorVulkan* getExecutorImpl(uintptr_t id);

struct RasterizerPipelineWrap {
    RasterizerPipelineVulkan* impl = nullptr;
    uint64_t refCount = 0;
};

static Corona::Kernel::Utils::Storage<RasterizerPipelineWrap> gRasterizerPipelineStorage;

static void incRaster(uintptr_t id) {
    if (id) {
        auto handle = gRasterizerPipelineStorage.acquire_write(id);
        ++handle->refCount;
    }
}
static void decRaster(uintptr_t id) {
    if (!id) {
        return;
    }
    bool destroy = false;
    auto handle = gRasterizerPipelineStorage.acquire_write(id);
    if (--handle->refCount == 0) {
        delete handle->impl;
        handle->impl = nullptr;
        destroy = true;
    }
    if (destroy) {
        gRasterizerPipelineStorage.deallocate(id);
    }
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
    incRaster(*rasterizerPipelineID);
}

RasterizerPipeline::~RasterizerPipeline() {
    if (rasterizerPipelineID) {
        decRaster(*rasterizerPipelineID);
    }
}

RasterizerPipeline& RasterizerPipeline::operator=(const RasterizerPipeline& other) {
    if (this != &other) {
        incRaster(*other.rasterizerPipelineID);
        decRaster(*rasterizerPipelineID);
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

RasterizerPipeline& RasterizerPipeline::record(const HardwareBuffer& indexBuffer) {
    auto handle = gRasterizerPipelineStorage.acquire_read(*rasterizerPipelineID);
    handle->impl->record(indexBuffer);
    return *this;
}

// ��ִ��������ʵ��
RasterizerPipelineVulkan* getRasterizerPipelineImpl(uintptr_t id) {
    RasterizerPipelineVulkan* ptr = nullptr;
    auto handle = gRasterizerPipelineStorage.acquire_read(id);
    ptr = handle->impl;
    return ptr;
}
