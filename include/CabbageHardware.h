#pragma once

#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

#include "Compiler/ShaderCodeCompiler.h"

// Forward declare platform-specific types instead of including platform headers
#if defined(_WIN32)
// Forward declare HANDLE without including Windows.h
#if !defined(_WINDEF_)
typedef void* HANDLE;
#endif
#endif

struct HardwareExecutor;

enum class ImageFormat : uint32_t {
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
    ASTC_4x4_SRGB,
};

enum class ImageUsage : uint32_t {
    SampledImage = 1,
    StorageImage = 2,
    DepthImage = 3,
};

enum class BufferUsage : uint32_t {
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

struct ExternalHandle {
#if _WIN32 || _WIN64
    HANDLE handle = nullptr;
#else
    int fd = -1;
#endif
};

// ================= 对外封装：HardwareBuffer =================
struct HardwareBuffer {
   public:
    HardwareBuffer();
    HardwareBuffer(const HardwareBuffer& other);
    HardwareBuffer(HardwareBuffer&& other) noexcept;
    HardwareBuffer(uint32_t bufferSize, uint32_t elementSize, BufferUsage usage, const void* data = nullptr);

    HardwareBuffer(uint32_t size, BufferUsage usage, const void* data = nullptr)
        : HardwareBuffer(1, size, usage, data) {
    }

    template <IsContainer Container>
    HardwareBuffer(const Container& input, BufferUsage usage)
        : HardwareBuffer(input.size(), sizeof(input[0]), usage, input.data()) {
    }

    HardwareBuffer(const ExternalHandle& memHandle, uint32_t bufferSize, uint32_t elementSize, uint32_t allocSize, BufferUsage usage);

    ~HardwareBuffer();

    HardwareBuffer& operator=(const HardwareBuffer& other);
    HardwareBuffer& operator=(HardwareBuffer&& other) noexcept;

    explicit operator bool() const;

    ExternalHandle exportBufferMemory();
    // HardwareBuffer importBufferMemory(const ExternalHandle &memHandle);

    [[nodiscard]] uint32_t storeDescriptor() const;

    bool copyFromBuffer(const HardwareBuffer& inputBuffer, const HardwareExecutor* executor) const;
    bool copyFromData(const void* inputData, uint64_t size) const;
    bool copyToData(void* outputData, uint64_t size) const;

    template <typename T>
    bool copyFromVector(const std::vector<T>& input) {
        return copyFromData(input.data(), input.size() * sizeof(T));
    }

    [[nodiscard]] void* getMappedData() const;
    [[nodiscard]] uint64_t getElementSize() const;
    [[nodiscard]] uint64_t getElementCount() const;

    [[nodiscard]] uintptr_t getBufferID() const {
        return bufferID.load(std::memory_order_acquire);
    }

   private:
    std::atomic<std::uintptr_t> bufferID;

    friend class HardwareImage;
};

// ================= HardwareImage 参数结构体 =================
struct HardwareImageCreateInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    ImageFormat format = ImageFormat::RGBA8_SRGB;
    ImageUsage usage = ImageUsage::SampledImage;
    int arrayLayers = 1;
    int mipLevels = 1;
    void* initialData = nullptr;

    HardwareImageCreateInfo() = default;
    HardwareImageCreateInfo(uint32_t w, uint32_t h, ImageFormat fmt = ImageFormat::RGBA8_SRGB)
        : width(w), height(h), format(fmt) {}
};

// ================= 对外封装：HardwareImage =================
struct HardwareImage {
   public:
    HardwareImage();
    HardwareImage(const HardwareImage& other);
    HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage = ImageUsage::SampledImage, int arrayLayers = 1, void* imageData = nullptr);
    HardwareImage(const HardwareImageCreateInfo& createInfo);

    ~HardwareImage();

    HardwareImage& operator=(const HardwareImage& other);
    explicit operator bool() const;

    [[nodiscard]] uint32_t storeDescriptor(uint32_t mipLevel = 0);

    [[nodiscard]] std::shared_ptr<uintptr_t> getImageID() const {
        return imageID;
    }

    [[nodiscard]] uint32_t getMipLevels() const;
    [[nodiscard]] std::pair<uint32_t, uint32_t> getMipLevelSize(uint32_t mipLevel) const;

    HardwareImage& copyFromBuffer(const HardwareBuffer& buffer, HardwareExecutor* executor, uint32_t mipLevel = 0);
    HardwareImage& copyFromData(const void* inputData, HardwareExecutor* executor, uint32_t mipLevel = 0);

   private:
    std::shared_ptr<uintptr_t> imageID;

    friend class HardwareDisplayer;
};

// ================= 对外封装：HardwarePushConstant =================
struct HardwarePushConstant {
   public:
    HardwarePushConstant();
    HardwarePushConstant(const HardwarePushConstant& other);

    template <typename T>
        requires(!std::is_same_v<std::remove_cvref_t<T>, HardwarePushConstant>)
    HardwarePushConstant(T data) {
        copyFromRaw(&data, sizeof(T));
    }

    // NOTE: the sub and whole must in the same thread
    HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole = nullptr);

    ~HardwarePushConstant();

    HardwarePushConstant& operator=(const HardwarePushConstant& other);

    // NOTE: must in the same thread
    [[nodiscard]] uint8_t* getData() const;
    [[nodiscard]] uint64_t getSize() const;
    [[nodiscard]] std::shared_ptr<uintptr_t> getPushConstantID() const {
        return pushConstantID;
    }

   private:
    void copyFromRaw(const void* src, uint64_t size);

    std::shared_ptr<uintptr_t> pushConstantID;
};

// ================= 对外封装：HardwareDisplayer =================
struct HardwareDisplayer {
   public:
    explicit HardwareDisplayer(void* surface = nullptr);
    HardwareDisplayer(const HardwareDisplayer& other);
    ~HardwareDisplayer();

    HardwareDisplayer& operator=(const HardwareDisplayer& other);
    HardwareDisplayer& operator<<(const HardwareImage& image);

    HardwareDisplayer& wait(const HardwareExecutor& executor);

    [[nodiscard]] std::shared_ptr<uintptr_t> getDisplayerID() const {
        return displaySurfaceID;
    }

   private:
    std::shared_ptr<uintptr_t> displaySurfaceID;
};

// ================= 对外封装：ComputePipeline =================
struct ComputePipeline {
   public:
    ComputePipeline();
    ComputePipeline(const std::string& shaderCode,
                    EmbeddedShader::ShaderLanguage language = EmbeddedShader::ShaderLanguage::GLSL,
                    const std::source_location& sourceLocation = std::source_location::current());
    ComputePipeline(const ComputePipeline& other);
    ~ComputePipeline();

    ComputePipeline& operator=(const ComputePipeline& other);

    std::variant<HardwarePushConstant> operator[](const std::string& resourceName);
    ComputePipeline& operator()(uint16_t x, uint16_t y, uint16_t z);

    [[nodiscard]] std::shared_ptr<uintptr_t> getComputePipelineID() const {
        return computePipelineID;
    }

   private:
    std::shared_ptr<uintptr_t> computePipelineID;
};

// ================= 对外封装：RasterizerPipeline =================
struct RasterizerPipeline {
   public:
    RasterizerPipeline();
    RasterizerPipeline(std::string vertexShaderCode,
                       std::string fragmentShaderCode,
                       uint32_t multiviewCount = 1,
                       EmbeddedShader::ShaderLanguage vertexShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                       EmbeddedShader::ShaderLanguage fragmentShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                       const std::source_location& sourceLocation = std::source_location::current());
    RasterizerPipeline(const RasterizerPipeline& other);
    ~RasterizerPipeline();

    RasterizerPipeline& operator=(const RasterizerPipeline& other);

    void setDepthImage(HardwareImage& depthImage);
    [[nodiscard]] HardwareImage getDepthImage();

    std::variant<HardwarePushConstant, HardwareBuffer, HardwareImage> operator[](const std::string& resourceName);
    RasterizerPipeline& operator()(uint16_t width, uint16_t height);
    RasterizerPipeline& record(const HardwareBuffer& indexBuffer, const HardwareBuffer& vertexBuffer);

    [[nodiscard]] std::shared_ptr<uintptr_t> getRasterizerPipelineID() const {
        return rasterizerPipelineID;
    }

   private:
    std::shared_ptr<uintptr_t> rasterizerPipelineID;
};

// ================= 对外封装：HardwareExecutor =================
struct HardwareExecutor {
   public:
    HardwareExecutor();
    HardwareExecutor(const HardwareExecutor& other);
    ~HardwareExecutor();

    HardwareExecutor& operator=(const HardwareExecutor& other);
    HardwareExecutor& operator<<(ComputePipeline& computePipeline);
    HardwareExecutor& operator<<(RasterizerPipeline& rasterizerPipeline);
    HardwareExecutor& operator<<(HardwareExecutor& other);

    HardwareExecutor& wait(HardwareExecutor& other);
    HardwareExecutor& commit();

    [[nodiscard]] std::shared_ptr<uintptr_t> getExecutorID() const {
        return executorID;
    }

   private:
    std::shared_ptr<uintptr_t> executorID;
};