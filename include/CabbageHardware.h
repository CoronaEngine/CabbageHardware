#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <type_traits>
#include <vector>

#include <ktm/ktm.h>
#include "Compiler/ShaderCodeCompiler.h"
#include "Codegen/ComputePipelineObject.h"
#include "HardwareCommands.h"

// Forward declare platform-specific types instead of including platform headers
#if defined(_WIN32)
// Forward declare HANDLE without including Windows.h
#if !defined(_WINDEF_)
typedef void *HANDLE;
#endif
#endif

struct HardwareExecutor;

enum class ImageFormat : uint32_t
{
    RGBA8_UINT,
    RGBA8_SINT,
    RGBA8_SRGB,

    RGBA16_UINT,
    RGBA16_SINT,
    RGBA16_FLOAT,

    RGBA32_UINT,
    RGBA32_SINT,
    RGBA32_FLOAT,

    RG32_FLOAT,

    D16_UNORM,
    D32_FLOAT,

    BC1_RGB_UNORM,
    BC1_RGB_SRGB,
    BC2_RGBA_UNORM,
    BC2_RGBA_SRGB,
    BC3_RGBA_UNORM,
    BC3_RGBA_SRGB,
    BC4_R_UNORM,
    BC4_R_SNORM,
    BC5_RG_UNORM,
    BC5_RG_SNORM,

    ASTC_4x4_UNORM,
    ASTC_4x4_SRGB
};

enum class ImageUsage : uint32_t
{
    SampledImage = 1,
    StorageImage = 2,
    DepthImage = 3,
};

enum class BufferUsage : uint32_t
{
    VertexBuffer = 1,
    IndexBuffer = 2,
    UniformBuffer = 4,
    StorageBuffer = 8,
};

template <typename T>
concept IsContainer = requires(T a) {
    { a.size() } -> std::convertible_to<size_t>;
    { a.data() };
    { a[0] };
};

struct ExternalHandle
{
#if _WIN32 || _WIN64
    HANDLE handle = nullptr;
#else
    int fd = -1;
#endif
};

// ================= 对外封装：HardwareBuffer =================
struct HardwareBuffer
{
  public:
    HardwareBuffer();
    HardwareBuffer(const HardwareBuffer &other);
    HardwareBuffer(HardwareBuffer &&other) noexcept;
    HardwareBuffer(uint32_t bufferSize, uint32_t elementSize, BufferUsage usage, const void *data = nullptr, bool useDedicated = true);

    HardwareBuffer(uint32_t size, BufferUsage usage, const void *data = nullptr, bool useDedicated = true)
        : HardwareBuffer(1, size, usage, data, useDedicated)
    {
    }

    template <IsContainer Container>
    HardwareBuffer(const Container &input, BufferUsage usage, bool useDedicated = true)
        : HardwareBuffer(input.size(), sizeof(input[0]), usage, input.data(), useDedicated)
    {
    }

    HardwareBuffer(const ExternalHandle &memHandle, uint32_t bufferSize, uint32_t elementSize, uint32_t allocSize, BufferUsage usage);

    ~HardwareBuffer();

    HardwareBuffer &operator=(const HardwareBuffer &other);
    HardwareBuffer &operator=(HardwareBuffer &&other) noexcept;

    explicit operator bool() const;

    ExternalHandle exportBufferMemory();
    // HardwareBuffer importBufferMemory(const ExternalHandle &memHandle);

    [[nodiscard]] uint32_t storeDescriptor() const;

    // 流式拷贝命令（用于 HardwareExecutor << ）
    [[nodiscard]] BufferCopyCommand copyTo(const HardwareBuffer &dst,
                                           uint64_t srcOffset = 0,
                                           uint64_t dstOffset = 0,
                                           uint64_t size = 0) const;
    [[nodiscard]] BufferToImageCommand copyTo(const HardwareImage &dst,
                                              uint64_t bufferOffset = 0,
                                              uint32_t imageLayer = 0,
                                              uint32_t imageMip = 0) const;

    // CPU 映射内存操作（非 GPU 命令）
    bool copyFromData(const void *inputData, uint64_t size) const;
    bool copyToData(void *outputData, uint64_t size) const;

    template <typename T>
    bool copyFromVector(const std::vector<T> &input)
    {
        return copyFromData(input.data(), input.size() * sizeof(T));
    }

    [[nodiscard]] void *getMappedData() const;
    [[nodiscard]] uint64_t getElementSize() const;
    [[nodiscard]] uint64_t getElementCount() const;

    [[nodiscard]] uintptr_t getBufferID() const
    {
        return bufferID.load(std::memory_order_acquire);
    }

  private:
    std::atomic<std::uintptr_t> bufferID;
    mutable std::mutex bufferMutex;

    friend class HardwareImage;
};

// ================= HardwareImage 参数结构体 =================
struct HardwareImageCreateInfo
{
    uint32_t width{0};
    uint32_t height{0};
    ImageFormat format = ImageFormat::RGBA8_SRGB;
    ImageUsage usage = ImageUsage::SampledImage;
    int arrayLayers{1};
    int mipLevels{1};
    // void *initialData{nullptr};

    HardwareImageCreateInfo() = default;
    HardwareImageCreateInfo(uint32_t w, uint32_t h, ImageFormat fmt = ImageFormat::RGBA8_SRGB)
        : width(w), height(h), format(fmt)
    {
    }
};

// ================= 对外封装：HardwareImage =================
struct HardwareImage
{
  public:
    HardwareImage();
    HardwareImage(const HardwareImage &other);
    HardwareImage(HardwareImage &&other) noexcept;
    HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage = ImageUsage::SampledImage, int arrayLayers = 1, void *imageData = nullptr);
    HardwareImage(const HardwareImageCreateInfo &createInfo);

    ~HardwareImage();

    HardwareImage &operator=(const HardwareImage &other);
    HardwareImage &operator=(HardwareImage &&other) noexcept;
    // image[layer][mip]
    HardwareImage operator[](const uint32_t index);
    explicit operator bool() const;

    [[nodiscard]] uint32_t storeDescriptor();
    [[nodiscard]] uintptr_t getImageID() const
    {
        return imageID.load(std::memory_order_acquire);
    }

    // 流式拷贝命令（用于 HardwareExecutor << ）
    [[nodiscard]] ImageCopyCommand copyTo(const HardwareImage &dst,
                                          uint32_t srcLayer = 0, uint32_t dstLayer = 0,
                                          uint32_t srcMip = 0, uint32_t dstMip = 0) const;
    [[nodiscard]] ImageToBufferCommand copyTo(const HardwareBuffer &dst,
                                              uint32_t imageLayer = 0,
                                              uint32_t imageMip = 0,
                                              uint64_t bufferOffset = 0) const;
    [[nodiscard]] BufferToImageCommand copyFrom(const void *inputData,
                                                uint32_t imageLayer = 0,
                                                uint32_t imageMip = 0) const;

    //[[nodiscard]] uint32_t getNumMipLevels() const;
    //[[nodiscard]] uint32_t getArrayLayers() const;

  private:
    std::atomic<std::uintptr_t> imageID;
    mutable std::mutex imageMutex;

    friend class HardwareDisplayer;
};

// ================= 对外封装：HardwarePushConstant =================
struct HardwarePushConstant
{
  public:
    HardwarePushConstant();
    HardwarePushConstant(const HardwarePushConstant &other);
    HardwarePushConstant(HardwarePushConstant &&other) noexcept;

    template <typename T>
        requires(!std::is_same_v<std::remove_cvref_t<T>, HardwarePushConstant>)
    explicit HardwarePushConstant(T data)
    {
        copyFromRaw(&data, sizeof(T));
    }

    // NOTE: the sub and whole must in the same thread
    HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant *whole = nullptr);

    ~HardwarePushConstant();

    HardwarePushConstant &operator=(const HardwarePushConstant &other);
    HardwarePushConstant &operator=(HardwarePushConstant &&other) noexcept;

    template <typename T>
        requires(!std::is_same_v<std::remove_cvref_t<T>, HardwarePushConstant>)
    HardwarePushConstant &operator=(const T &data)
    {
        copyFromRaw(&data, sizeof(T));
        return *this;
    }

    template <typename T>
        requires(!std::is_same_v<std::remove_cvref_t<T>, HardwarePushConstant>)
    HardwarePushConstant &operator=(T &&data)
    {
        copyFromRaw(&data, sizeof(T));
        return *this;
    }

    // NOTE: must in the same thread
    [[nodiscard]] uint8_t *getData() const;
    [[nodiscard]] uint64_t getSize() const;
    [[nodiscard]] uintptr_t getPushConstantID() const
    {
        return pushConstantID.load(std::memory_order_acquire);
    }

  private:
    void copyFromRaw(const void *src, uint64_t size);

    std::atomic<std::uintptr_t> pushConstantID;
    mutable std::mutex pushConstantMutex;
};

// Forward declarations
struct ComputePipeline;
struct RasterizerPipeline;

// ================= 资源代理类：ResourceProxy =================
// 支持透明读写访问 Pipeline 内部资源
struct ResourceProxy
{
  private:
    ComputePipeline *compute_pipeline_{nullptr};
    RasterizerPipeline *rasterizer_pipeline_{nullptr};
    std::string resource_name_;

  public:
    ResourceProxy(ComputePipeline *p, std::string name)
        : compute_pipeline_(p), resource_name_(std::move(name))
    {
    }

    ResourceProxy(RasterizerPipeline *p, std::string name)
        : rasterizer_pipeline_(p), resource_name_(std::move(name))
    {
    }

    // ========== 赋值操作（写回内部） ==========

    template <typename T>
    ResourceProxy &operator=(const T &value);

    // ========== 显式转换（读取） ==========

    explicit operator HardwareImage() const;
    explicit operator HardwareBuffer() const;
    explicit operator HardwarePushConstant() const;

    // ========== 显式获取引用 ==========

    [[nodiscard]] HardwareImage asImage() const;
    [[nodiscard]] HardwareBuffer asBuffer() const;
    [[nodiscard]] HardwarePushConstant asPushConstant() const;
};

// ================= 对外封装：HardwareDisplayer =================
struct HardwareDisplayer
{
  public:
    explicit HardwareDisplayer(void *surface = nullptr);
    HardwareDisplayer(const HardwareDisplayer &other);
    HardwareDisplayer(HardwareDisplayer &&other) noexcept;
    ~HardwareDisplayer();

    HardwareDisplayer &operator=(const HardwareDisplayer &other);
    HardwareDisplayer &operator=(HardwareDisplayer &&other) noexcept;
    HardwareDisplayer &operator<<(const HardwareImage &image);

    HardwareDisplayer &wait(const HardwareExecutor &executor);

    [[nodiscard]] uintptr_t getDisplayerID() const
    {
        return displaySurfaceID.load(std::memory_order_acquire);
    }

  private:
    std::atomic<std::uintptr_t> displaySurfaceID;
    mutable std::mutex displayerMutex;
};

// ================= 对外封装：ComputePipeline =================
struct ComputePipeline
{
  public:
    ComputePipeline();
    ComputePipeline(const std::string &shaderCode,
                    EmbeddedShader::ShaderLanguage language = EmbeddedShader::ShaderLanguage::GLSL,
                    const std::source_location &sourceLocation = std::source_location::current());

    // 模板构造函数：接受 DSL lambda，内部完成编译
    template <typename F>
        requires std::invocable<F> && (!std::is_convertible_v<F, std::string>)
    ComputePipeline(F &&computeShaderCode,
                    ktm::uvec3 numthreads = ktm::uvec3(1),
                    EmbeddedShader::CompilerOption compilerOption = {},
                    std::source_location sourceLocation = std::source_location::current());

    ComputePipeline(const ComputePipeline &other);
    ComputePipeline(ComputePipeline &&other) noexcept;
    ~ComputePipeline();

    ComputePipeline &operator=(const ComputePipeline &other);
    ComputePipeline &operator=(ComputePipeline &&other) noexcept;

    ResourceProxy operator[](const std::string &resourceName);
    ComputePipeline &operator()(uint16_t x, uint16_t y, uint16_t z);

    void setPushConstant(const std::string &name, const void *data, size_t size);
    void setResource(const std::string &name, const HardwareBuffer &buffer);
    void setResource(const std::string &name, const HardwareImage &image);

    [[nodiscard]] HardwarePushConstant getPushConstant(const std::string &name) const;
    [[nodiscard]] HardwareBuffer getBuffer(const std::string &name) const;
    [[nodiscard]] HardwareImage getImage(const std::string &name) const;

    [[nodiscard]] uintptr_t getComputePipelineID() const
    {
        return computePipelineID.load(std::memory_order_acquire);
    }

  private:
    mutable std::mutex computePipelineMutex;
    std::atomic<std::uintptr_t> computePipelineID;
};

// ================= 对外封装：RasterizerPipeline =================
struct RasterizerPipeline
{
  public:
    RasterizerPipeline();
    RasterizerPipeline(std::string vertexShaderCode,
                       std::string fragmentShaderCode,
                       uint32_t multiviewCount = 1,
                       EmbeddedShader::ShaderLanguage vertexShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                       EmbeddedShader::ShaderLanguage fragmentShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                       const std::source_location &sourceLocation = std::source_location::current());
    RasterizerPipeline(const RasterizerPipeline &other);
    RasterizerPipeline(RasterizerPipeline &&other) noexcept;
    ~RasterizerPipeline();

    RasterizerPipeline &operator=(const RasterizerPipeline &other);
    RasterizerPipeline &operator=(RasterizerPipeline &&other) noexcept;

    void setDepthImage(HardwareImage &depthImage);
    [[nodiscard]] HardwareImage getDepthImage();

    ResourceProxy operator[](const std::string &resourceName);
    RasterizerPipeline &operator()(uint16_t width, uint16_t height);
    RasterizerPipeline &record(const HardwareBuffer &indexBuffer, const HardwareBuffer &vertexBuffer);

    void setPushConstant(const std::string &name, const void *data, size_t size);
    void setResource(const std::string &name, const HardwareBuffer &buffer);
    void setResource(const std::string &name, const HardwareImage &image);

    [[nodiscard]] HardwarePushConstant getPushConstant(const std::string &name) const;
    [[nodiscard]] HardwareBuffer getBuffer(const std::string &name) const;
    [[nodiscard]] HardwareImage getImage(const std::string &name) const;

    [[nodiscard]] uintptr_t getRasterizerPipelineID() const
    {
        return rasterizerPipelineID.load(std::memory_order_acquire);
    }

  private:
    mutable std::mutex rasterizerPipelineMutex;
    std::atomic<std::uintptr_t> rasterizerPipelineID;
};

// ================= 对外封装：HardwareExecutor =================
struct HardwareExecutor
{
  public:
    HardwareExecutor();
    HardwareExecutor(const HardwareExecutor &other);
    HardwareExecutor(HardwareExecutor &&other) noexcept;
    ~HardwareExecutor();

    HardwareExecutor &operator=(const HardwareExecutor &other);
    HardwareExecutor &operator=(HardwareExecutor &&other) noexcept;

    HardwareExecutor &operator<<(ComputePipeline &computePipeline);
    HardwareExecutor &operator<<(RasterizerPipeline &rasterizerPipeline);
    HardwareExecutor &operator<<(HardwareExecutor &other);
    HardwareExecutor &operator<<(const CopyCommand &cmd);

    HardwareExecutor &wait(HardwareExecutor &other);
    HardwareExecutor &commit();

    // ========== 延迟释放相关接口 ==========
    /// @brief 等待所有延迟释放的资源完成（阻塞）
    void waitForDeferredResources();

    /// @brief 手动触发一次清理（非阻塞）
    void cleanupDeferredResources();

    [[nodiscard]] uintptr_t getExecutorID() const
    {
        return executorID.load(std::memory_order_acquire);
    }

  private:
    mutable std::mutex executorMutex;
    std::atomic<std::uintptr_t> executorID;
};

// ================= ResourceProxy Implementation =================

template <typename T>
ResourceProxy &ResourceProxy::operator=(const T &value)
{
    if constexpr (std::is_same_v<std::remove_cvref_t<T>, HardwareImage>)
    {
        if (compute_pipeline_)
            compute_pipeline_->setResource(resource_name_, value);
        if (rasterizer_pipeline_)
            rasterizer_pipeline_->setResource(resource_name_, value);
    }
    else if constexpr (std::is_same_v<std::remove_cvref_t<T>, HardwareBuffer>)
    {
        if (compute_pipeline_)
            compute_pipeline_->setResource(resource_name_, value);
        if (rasterizer_pipeline_)
            rasterizer_pipeline_->setResource(resource_name_, value);
    }
    else if constexpr (!std::is_same_v<std::remove_cvref_t<T>, ResourceProxy>)
    {
        if (compute_pipeline_)
            compute_pipeline_->setPushConstant(resource_name_, &value, sizeof(T));
        if (rasterizer_pipeline_)
            rasterizer_pipeline_->setPushConstant(resource_name_, &value, sizeof(T));
    }
    return *this;
}

inline ResourceProxy::operator HardwareImage() const
{
    return asImage();
}

inline ResourceProxy::operator HardwareBuffer() const
{
    return asBuffer();
}

inline ResourceProxy::operator HardwarePushConstant() const
{
    return asPushConstant();
}

inline HardwareImage ResourceProxy::asImage() const
{
    if (compute_pipeline_)
        return compute_pipeline_->getImage(resource_name_);
    if (rasterizer_pipeline_)
        return rasterizer_pipeline_->getImage(resource_name_);
    throw std::runtime_error("ResourceProxy not initialized");
}

inline HardwareBuffer ResourceProxy::asBuffer() const
{
    if (compute_pipeline_)
        return compute_pipeline_->getBuffer(resource_name_);
    if (rasterizer_pipeline_)
        return rasterizer_pipeline_->getBuffer(resource_name_);
    throw std::runtime_error("ResourceProxy not initialized");
}

inline HardwarePushConstant ResourceProxy::asPushConstant() const
{
    if (compute_pipeline_)
        return compute_pipeline_->getPushConstant(resource_name_);
    if (rasterizer_pipeline_)
        return rasterizer_pipeline_->getPushConstant(resource_name_);
    throw std::runtime_error("ResourceProxy not initialized");
}

// ================= ComputePipeline 模板构造函数实现 =================
// 需要访问 Storage 和 ComputePipelineVulkan，通过辅助函数实现
void computePipelineInitFromCompiler(std::atomic<std::uintptr_t> &pipelineID, 
                                      const EmbeddedShader::ShaderCodeCompiler &compiler,
                                      const std::source_location &src);

template <typename F>
    requires std::invocable<F> && (!std::is_convertible_v<F, std::string>)
ComputePipeline::ComputePipeline(F &&computeShaderCode,
                                  ktm::uvec3 numthreads,
                                  EmbeddedShader::CompilerOption compilerOption,
                                  std::source_location sourceLocation)
{
    // 使用 helicon 编译 DSL lambda 到着色器代码
    auto pipelineObj = EmbeddedShader::ComputePipelineObject::compile(
        std::forward<F>(computeShaderCode),
        numthreads,
        compilerOption,
        sourceLocation
    );

    // 调用辅助函数完成 Vulkan 管线创建
    computePipelineInitFromCompiler(computePipelineID, *pipelineObj.compute, sourceLocation);
}
