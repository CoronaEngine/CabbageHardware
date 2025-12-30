#pragma once

#include <memory>
#include <type_traits>
#include <atomic>
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
    HardwareBuffer(uint32_t bufferSize, uint32_t elementSize, BufferUsage usage, const void* data = nullptr, bool useDedicated = true);

    HardwareBuffer(uint32_t size, BufferUsage usage, const void* data = nullptr, bool useDedicated = true)
        : HardwareBuffer(1, size, usage, data, useDedicated) {
    }

    template <IsContainer Container>
    HardwareBuffer(const Container& input, BufferUsage usage, bool useDedicated = true)
        : HardwareBuffer(input.size(), sizeof(input[0]), usage, input.data(), useDedicated) {
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
    uint32_t width{0};
    uint32_t height{0};
    ImageFormat format = ImageFormat::RGBA8_SRGB;
    ImageUsage usage = ImageUsage::SampledImage;
    int arrayLayers{1};
    int mipLevels{1};
    void* initialData{nullptr};

    HardwareImageCreateInfo() = default;
    HardwareImageCreateInfo(uint32_t w, uint32_t h, ImageFormat fmt = ImageFormat::RGBA8_SRGB)
        : width(w), height(h), format(fmt) {}
};

// ================= 对外封装：HardwareImage =================
struct HardwareImage {
   public:
    HardwareImage();
    HardwareImage(const HardwareImage& other);
    HardwareImage(HardwareImage&& other) noexcept;
    HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage = ImageUsage::SampledImage, int arrayLayers = 1, void* imageData = nullptr);
    HardwareImage(const HardwareImageCreateInfo& createInfo);

    ~HardwareImage();

    HardwareImage& operator=(const HardwareImage& other);
    HardwareImage& operator=(HardwareImage&& other) noexcept;
    // image[layer][mip]
    HardwareImage operator[](const uint32_t index);
    explicit operator bool() const;

    [[nodiscard]] uint32_t storeDescriptor();
    [[nodiscard]] uintptr_t getImageID() const {
        return imageID.load(std::memory_order_acquire);
    }

    //[[nodiscard]] uint32_t getNumMipLevels() const;
    //[[nodiscard]] uint32_t getArrayLayers() const;

    HardwareImage& copyFromBuffer(const HardwareBuffer& buffer, HardwareExecutor* executor, uint32_t mipLevel = 0);
    HardwareImage& copyFromData(const void* inputData, HardwareExecutor* executor, uint32_t mipLevel = 0);

   private:
    std::atomic<std::uintptr_t> imageID;

    friend class HardwareDisplayer;
};

// ================= 对外封装：HardwarePushConstant =================
struct HardwarePushConstant {
   public:
    HardwarePushConstant();
    HardwarePushConstant(const HardwarePushConstant& other);
    HardwarePushConstant(HardwarePushConstant&& other) noexcept;

    template <typename T>
        requires(!std::is_same_v<std::remove_cvref_t<T>, HardwarePushConstant>)
    explicit HardwarePushConstant(T data) {
        copyFromRaw(&data, sizeof(T));
    }

    // NOTE: the sub and whole must in the same thread
    HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole = nullptr);

    ~HardwarePushConstant();

    HardwarePushConstant& operator=(const HardwarePushConstant& other);
    HardwarePushConstant& operator=(HardwarePushConstant&& other) noexcept;

    template <typename T>
        requires(!std::is_same_v<std::remove_cvref_t<T>, HardwarePushConstant>)
    HardwarePushConstant& operator=(const T& data) {
        copyFromRaw(&data, sizeof(T));
        return *this;
    }

    template <typename T>
        requires(!std::is_same_v<std::remove_cvref_t<T>, HardwarePushConstant>)
    HardwarePushConstant& operator=(T&& data) {
        copyFromRaw(&data, sizeof(T));
        return *this;
    }

    // NOTE: must in the same thread
    [[nodiscard]] uint8_t* getData() const;
    [[nodiscard]] uint64_t getSize() const;
    [[nodiscard]] uintptr_t getPushConstantID() const {
        return pushConstantID.load(std::memory_order_acquire);
    }

   private:
    void copyFromRaw(const void* src, uint64_t size);

    std::atomic<std::uintptr_t> pushConstantID;
};

// ================= 资源代理类：ResourceProxy =================
// 支持透明读写访问 Pipeline 内部资源
struct ResourceProxy {
    enum class Type { kPushConstant,
                      kBuffer,
                      kImage };

   private:
    Type type_;
    HardwarePushConstant push_constant_{nullptr};
    HardwareBuffer* buffer_ptr_{nullptr};
    HardwareImage* image_ptr_{nullptr};

   public:
    // PushConstant - 直接存储值（已有写回机制）
    explicit ResourceProxy(HardwarePushConstant push_constant)
        : type_(Type::kPushConstant), push_constant_(std::move(push_constant)) {}

    // Buffer - 存储指针
    explicit ResourceProxy(HardwareBuffer* buffer)
        : type_(Type::kBuffer), buffer_ptr_(buffer) {}

    // Image - 存储指针
    explicit ResourceProxy(HardwareImage* image)
        : type_(Type::kImage), image_ptr_(image) {}

    // ========== 赋值操作（写回内部） ==========

    template <typename T>
    ResourceProxy& operator=(const T& value) {
        if constexpr (std::is_same_v<std::remove_cvref_t<T>, HardwareImage>) {
            if (type_ == Type::kImage && image_ptr_ != nullptr) {
                *image_ptr_ = value;
            }
        } else if constexpr (std::is_same_v<std::remove_cvref_t<T>, HardwareBuffer>) {
            if (type_ == Type::kBuffer && buffer_ptr_ != nullptr) {
                *buffer_ptr_ = value;
            }
        } else if constexpr (!std::is_same_v<std::remove_cvref_t<T>, ResourceProxy>) {
            if (type_ == Type::kPushConstant) {
                push_constant_ = value;
            }
        }
        return *this;
    }

    // ========== 移动赋值操作 ==========

    template <typename T>
        requires(!std::is_lvalue_reference_v<T>)  // 约束必须为右值，防止自动推导为引用类型
    ResourceProxy& operator=(T&& value) {
        if constexpr (std::is_same_v<std::remove_cvref_t<T>, HardwareImage>) {
            if (type_ == Type::kImage && image_ptr_ != nullptr) {
                *image_ptr_ = std::move(value);
            }
        } else if constexpr (std::is_same_v<std::remove_cvref_t<T>, HardwareBuffer>) {
            if (type_ == Type::kBuffer && buffer_ptr_ != nullptr) {
                *buffer_ptr_ = std::move(value);
            }
        } else if constexpr (!std::is_same_v<std::remove_cvref_t<T>, ResourceProxy>) {
            if (type_ == Type::kPushConstant) {
                push_constant_ = std::move(value);
            }
        }
        return *this;
    }

    // ========== 显式转换（读取） ==========

    explicit operator HardwareImage() const {
        return (image_ptr_ != nullptr) ? *image_ptr_ : HardwareImage();
    }

    explicit operator HardwareBuffer() const {
        return (buffer_ptr_ != nullptr) ? *buffer_ptr_ : HardwareBuffer();
    }

    explicit operator HardwarePushConstant() const {
        return push_constant_;
    }

    // ========== 类型查询 ==========

    [[nodiscard]] Type getType() const { return type_; }
    [[nodiscard]] bool isImage() const { return type_ == Type::kImage; }
    [[nodiscard]] bool isBuffer() const { return type_ == Type::kBuffer; }
    [[nodiscard]] bool isPushConstant() const { return type_ == Type::kPushConstant; }

    // ========== 显式获取引用 ==========

    [[nodiscard]] HardwareImage& asImage() {
        if (type_ != Type::kImage || image_ptr_ == nullptr) {
            throw std::runtime_error("Resource is not an Image");
        }
        return *image_ptr_;
    }

    [[nodiscard]] HardwareBuffer& asBuffer() {
        if (type_ != Type::kBuffer || buffer_ptr_ == nullptr) {
            throw std::runtime_error("Resource is not a Buffer");
        }
        return *buffer_ptr_;
    }

    [[nodiscard]] HardwarePushConstant& asPushConstant() {
        if (type_ != Type::kPushConstant) {
            throw std::runtime_error("Resource is not a PushConstant");
        }
        return push_constant_;
    }
};

// ================= 对外封装：HardwareDisplayer =================
struct HardwareDisplayer {
   public:
    explicit HardwareDisplayer(void* surface = nullptr);
    HardwareDisplayer(const HardwareDisplayer& other);
    HardwareDisplayer(HardwareDisplayer&& other) noexcept;
    ~HardwareDisplayer();

    HardwareDisplayer& operator=(const HardwareDisplayer& other);
    HardwareDisplayer& operator=(HardwareDisplayer&& other) noexcept;
    HardwareDisplayer& operator<<(const HardwareImage& image);

    HardwareDisplayer& wait(const HardwareExecutor& executor);

    [[nodiscard]] uintptr_t getDisplayerID() const {
        return displaySurfaceID.load(std::memory_order_acquire);
    }

   private:
    std::atomic<std::uintptr_t> displaySurfaceID;
};

// ================= 对外封装：ComputePipeline =================
struct ComputePipeline {
   public:
    ComputePipeline();
    ComputePipeline(const std::string& shaderCode,
                    EmbeddedShader::ShaderLanguage language = EmbeddedShader::ShaderLanguage::GLSL,
                    const std::source_location& sourceLocation = std::source_location::current());
    ComputePipeline(const ComputePipeline& other);
    ComputePipeline(ComputePipeline&& other) noexcept;
    ~ComputePipeline();

    ComputePipeline& operator=(const ComputePipeline& other);
    ComputePipeline& operator=(ComputePipeline&& other) noexcept;

    ResourceProxy operator[](const std::string& resourceName);
    ComputePipeline& operator()(uint16_t x, uint16_t y, uint16_t z);

    [[nodiscard]] uintptr_t getComputePipelineID() const {
        return computePipelineID.load(std::memory_order_acquire);
    }

   private:
    std::atomic<std::uintptr_t> computePipelineID;
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
    RasterizerPipeline(RasterizerPipeline&& other) noexcept;
    ~RasterizerPipeline();

    RasterizerPipeline& operator=(const RasterizerPipeline& other);
    RasterizerPipeline& operator=(RasterizerPipeline&& other) noexcept;

    void setDepthImage(HardwareImage& depthImage);
    [[nodiscard]] HardwareImage getDepthImage();

    ResourceProxy operator[](const std::string& resourceName);
    RasterizerPipeline& operator()(uint16_t width, uint16_t height);
    RasterizerPipeline& record(const HardwareBuffer& indexBuffer, const HardwareBuffer& vertexBuffer);

    [[nodiscard]] uintptr_t getRasterizerPipelineID() const {
        return rasterizerPipelineID.load(std::memory_order_acquire);
    }

   private:
    std::atomic<std::uintptr_t> rasterizerPipelineID;
};

// ================= 对外封装：HardwareExecutor =================
struct HardwareExecutor {
   public:
    HardwareExecutor();
    HardwareExecutor(const HardwareExecutor& other);
    HardwareExecutor(HardwareExecutor&& other) noexcept;
    ~HardwareExecutor();

    HardwareExecutor& operator=(const HardwareExecutor& other);
    HardwareExecutor& operator=(HardwareExecutor&& other) noexcept;

    HardwareExecutor& operator<<(ComputePipeline& computePipeline);
    HardwareExecutor& operator<<(RasterizerPipeline& rasterizerPipeline);
    HardwareExecutor& operator<<(HardwareExecutor& other);

    HardwareExecutor& wait(HardwareExecutor& other);
    HardwareExecutor& commit();

    [[nodiscard]] uintptr_t getExecutorID() const {
        return executorID.load(std::memory_order_acquire);
    }

   private:
    std::atomic<std::uintptr_t> executorID;
};