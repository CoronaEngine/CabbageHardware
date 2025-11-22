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

struct HardwareBuffer {
   public:
    HardwareBuffer();
    HardwareBuffer(const HardwareBuffer& other);
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

    [[nodiscard]] std::shared_ptr<uintptr_t> getBufferID() const {
        return bufferID;
    }

   private:
    std::shared_ptr<uintptr_t> bufferID;

    friend class HardwareImage;
};

struct HardwareImage {
   public:
    HardwareImage();
    HardwareImage(const HardwareImage& other);
    HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage = ImageUsage::SampledImage, int arrayLayers = 1, void* imageData = nullptr);

    ~HardwareImage();

    HardwareImage& operator=(const HardwareImage& other);
    explicit operator bool() const;

    [[nodiscard]] uint32_t storeDescriptor();

    [[nodiscard]] std::shared_ptr<uintptr_t> getImageID() const {
        return imageID;
    }

    HardwareImage& copyFromBuffer(const HardwareBuffer& buffer, HardwareExecutor* executor);
    HardwareImage& copyFromData(const void* inputData, HardwareExecutor* executor);

   private:
    std::shared_ptr<uintptr_t> imageID;

    friend class HardwareDisplayer;
};

struct HardwarePushConstant {
   public:
    HardwarePushConstant();
    HardwarePushConstant(const HardwarePushConstant& other);

    template <typename T>
    HardwarePushConstant(T data)
        requires(!std::is_same_v<std::remove_cvref_t<T>, HardwarePushConstant>)
    {
        copyFromRaw(&data, sizeof(T));
    }

    // the sub and whole must in the same thread
    HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole = nullptr);

    ~HardwarePushConstant();

    HardwarePushConstant& operator=(const HardwarePushConstant& other);

    // must in the same thread
    [[nodiscard]] uint8_t* getData() const;

    [[nodiscard]] uint64_t getSize() const;

    [[nodiscard]] std::shared_ptr<uintptr_t> getPushConstantID() const {
        return pushConstantID;
    }

   private:
    void copyFromRaw(const void* src, uint64_t size);

    std::shared_ptr<uintptr_t> pushConstantID;
};

struct HardwareDisplayer {
   public:
    explicit HardwareDisplayer(void* surface = nullptr);
    HardwareDisplayer(const HardwareDisplayer& other);
    ~HardwareDisplayer();

    HardwareDisplayer& operator=(const HardwareDisplayer& other);
    HardwareDisplayer& operator<<(HardwareImage& image);

    HardwareDisplayer& wait(HardwareExecutor& executor);

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
    std::shared_ptr<uintptr_t> computePipelineID;  // 内部存储句柄
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
    RasterizerPipeline& record(const HardwareBuffer& indexBuffer);

    [[nodiscard]] std::shared_ptr<uintptr_t> getRasterizerPipelineID() const {
        return rasterizerPipelineID;
    }

   private:
    std::shared_ptr<uintptr_t> rasterizerPipelineID;  // 内部存储句柄
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
    HardwareExecutor& operator<<(HardwareExecutor& other);  // 透传

    HardwareExecutor& wait(HardwareExecutor& other);
    HardwareExecutor& commit();

    [[nodiscard]] std::shared_ptr<uintptr_t> getExecutorID() const {
        return executorID;
    }

   private:
    std::shared_ptr<uintptr_t> executorID;  // 内部存储句柄
};