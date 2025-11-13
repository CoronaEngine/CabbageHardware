#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <type_traits>

//#include <ktm/ktm.h>

#include "Hardware/HardwareExecutor.h"

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

struct HardwareBuffer
{
public:
    HardwareBuffer();
    HardwareBuffer(const HardwareBuffer &other);
    HardwareBuffer(uint32_t bufferSize, uint32_t elementSize, BufferUsage usage, const void *data = nullptr);

    HardwareBuffer(uint32_t size, BufferUsage usage, const void *data = nullptr)
        : HardwareBuffer(1, size, usage, data)
    {
    }

    template <IsContainer Container>
    HardwareBuffer(const Container &input, BufferUsage usage)
        : HardwareBuffer(input.size(), sizeof(input[0]), usage, input.data())
    {
    }

    ~HardwareBuffer();

    HardwareBuffer &operator=(const HardwareBuffer &other);
    explicit operator bool() const;

    ExternalHandle exportBufferMemory();
    HardwareBuffer importBufferMemory(const ExternalHandle &memHandle);

    [[nodiscard]] uint32_t storeDescriptor();

    bool copyFromBuffer(const HardwareBuffer &inputBuffer, HardwareExecutor *executor);

    bool copyFromData(const void *inputData, uint64_t size);

    template <typename T>
    bool copyFromVector(const std::vector<T> &input)
    {
        return copyFromData(input.data(), input.size() * sizeof(T));
    }

    [[nodiscard]] void *getMappedData();

    [[nodiscard]] uint64_t getBufferSize() const;

    [[nodiscard]] std::shared_ptr<uintptr_t> getBufferID() const
    {
        return bufferID;
    }

private:
    std::shared_ptr<uintptr_t> bufferID;

    friend class HardwareImage;
};

struct HardwareImage
{
public:
    HardwareImage();
    HardwareImage(const HardwareImage &other);
    HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage = ImageUsage::SampledImage, int arrayLayers = 1, void *imageData = nullptr);

    ~HardwareImage();

    HardwareImage &operator=(const HardwareImage &other);
    explicit operator bool() const;

    [[nodiscard]] uint32_t storeDescriptor();

    [[nodiscard]] std::shared_ptr<uintptr_t> getImageID() const
    {
        return imageID;
    }

private:
    HardwareImage &copyFromBuffer(const HardwareBuffer &buffer);

    HardwareImage &copyFromData(const void *inputData);

    std::shared_ptr<uintptr_t> imageID;

    friend class HardwareDisplayer;
};

struct HardwarePushConstant
{
public:
    HardwarePushConstant();
    HardwarePushConstant(const HardwarePushConstant &other);

    template <typename T>
    HardwarePushConstant(T data)
        requires(!std::is_same_v<std::remove_cvref_t<T>, HardwarePushConstant>)
    {
        copyFromRaw(&data, sizeof(T));
    }

    HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant *whole = nullptr);

    ~HardwarePushConstant();

    HardwarePushConstant &operator=(const HardwarePushConstant &other);

    [[nodiscard]] uint8_t *getData() const;

    [[nodiscard]] uint64_t getSize() const;

    [[nodiscard]] std::shared_ptr<uintptr_t> getPushConstantID() const
    {
        return pushConstantID;
    }

private:
    void copyFromRaw(const void *src, uint64_t size);

    std::shared_ptr<uintptr_t> pushConstantID;
};

struct HardwareDisplayer
{
public:
    explicit HardwareDisplayer(void *surface = nullptr);
    HardwareDisplayer(const HardwareDisplayer &other);
    ~HardwareDisplayer();

    HardwareDisplayer &operator=(const HardwareDisplayer &other);
    HardwareDisplayer &operator<<(HardwareImage &image);

    HardwareDisplayer &wait(HardwareExecutor &executor);

    [[nodiscard]] std::shared_ptr<uintptr_t> getDisplayerID() const
    {
        return displaySurfaceID;
    }

private:
    std::shared_ptr<uintptr_t> displaySurfaceID;
};