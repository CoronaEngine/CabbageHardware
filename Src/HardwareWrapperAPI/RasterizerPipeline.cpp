#include "Pipeline/RasterizerPipeline.h"
#include "CabbageHardware.h"

HardwareExecutorVulkan *getExecutorImpl(uintptr_t id);

struct RasterizerPipelineWrap
{
    RasterizerPipelineVulkan *impl = nullptr;
    uint64_t refCount = 0;
};

static Corona::Kernel::Utils::Storage<RasterizerPipelineWrap> gRasterizerPipelineStorage;

static void incRaster(uintptr_t id)
{
    if (id)
        gRasterizerPipelineStorage.write(id, [](RasterizerPipelineWrap &w) { ++w.refCount; });
}
static void decRaster(uintptr_t id)
{
    if (!id)
        return;
    bool destroy = false;
    gRasterizerPipelineStorage.write(id, [&](RasterizerPipelineWrap &w) { if(--w.refCount==0){ delete w.impl; w.impl=nullptr; destroy=true; } });
    if (destroy)
        gRasterizerPipelineStorage.deallocate(id);
}

RasterizerPipeline::RasterizerPipeline()
{
    const auto handle = gRasterizerPipelineStorage.allocate([](RasterizerPipelineWrap &w) { w.impl = new RasterizerPipelineVulkan(); w.refCount=1; });
    rasterizerPipelineID = std::make_shared<uintptr_t>(handle);
}

RasterizerPipeline::RasterizerPipeline(std::string vs, std::string fs, uint32_t multiviewCount, EmbeddedShader::ShaderLanguage vlang, EmbeddedShader::ShaderLanguage flang, const std::source_location &src)
{
    const auto handle = gRasterizerPipelineStorage.allocate([&](RasterizerPipelineWrap &w) { w.impl = new RasterizerPipelineVulkan(vs, fs, multiviewCount, vlang, flang, src); w.refCount=1; });
    rasterizerPipelineID = std::make_shared<uintptr_t>(handle);
}

RasterizerPipeline::RasterizerPipeline(const RasterizerPipeline &other)
    : rasterizerPipelineID(other.rasterizerPipelineID)
{
    incRaster(*rasterizerPipelineID);
}

RasterizerPipeline::~RasterizerPipeline()
{
    if (rasterizerPipelineID)
        decRaster(*rasterizerPipelineID);
}

RasterizerPipeline &RasterizerPipeline::operator=(const RasterizerPipeline &other)
{
    if (this != &other)
    {
        incRaster(*other.rasterizerPipelineID);
        decRaster(*rasterizerPipelineID);
        *rasterizerPipelineID = *other.rasterizerPipelineID;
    }
    return *this;
}

void RasterizerPipeline::setDepthImage(HardwareImage &depthImage)
{
    gRasterizerPipelineStorage.read(*rasterizerPipelineID, [&](const RasterizerPipelineWrap &w) { w.impl->setDepthImage(depthImage); });
}

HardwareImage RasterizerPipeline::getDepthImage()
{
    HardwareImage img;
    gRasterizerPipelineStorage.read(*rasterizerPipelineID, [&](const RasterizerPipelineWrap &w) { img = w.impl->getDepthImage(); });
    return img;
}

std::variant<HardwarePushConstant, HardwareBuffer, HardwareImage> RasterizerPipeline::operator[](const std::string &resourceName)
{
    std::variant<HardwarePushConstant, HardwareBuffer, HardwareImage> r;
    gRasterizerPipelineStorage.read(*rasterizerPipelineID, [&](const RasterizerPipelineWrap &w) { r = (*w.impl)[resourceName]; });
    return r;
}

RasterizerPipeline &RasterizerPipeline::operator()(uint16_t width, uint16_t height)
{
    gRasterizerPipelineStorage.read(*rasterizerPipelineID, [&](const RasterizerPipelineWrap &w) { (*w.impl)(width, height); });
    return *this;
}

RasterizerPipeline &RasterizerPipeline::record(const HardwareBuffer &indexBuffer)
{
    gRasterizerPipelineStorage.read(*rasterizerPipelineID, [&](const RasterizerPipelineWrap &w) { w.impl->record(indexBuffer); });
    return *this;
}

// 供执行器访问实现
RasterizerPipelineVulkan *getRasterizerPipelineImpl(uintptr_t id)
{
    RasterizerPipelineVulkan *ptr = nullptr;
    gRasterizerPipelineStorage.read(id, [&](const RasterizerPipelineWrap &w) { ptr = w.impl; });
    return ptr;
}
