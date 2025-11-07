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
    D32_FLOAT, // just for depth
};

enum class ImageUsage : uint32_t
{
    SampledImage = 1,
    StorageImage = 2,
    DepthImage = 3,
};

enum class BufferUsage : int32_t
{
    VertexBuffer = 1,
    IndexBuffer = 2, // must be uint32_t
    UniformBuffer = 4,
    StorageBuffer = 8,
};

template <typename T>
concept is_container = requires(T a) {
    a.size();
    a.data();
    a[0];
};

struct HardwareBuffer
{
    HardwareBuffer();
    HardwareBuffer(const HardwareBuffer &other);
    HardwareBuffer(uint64_t size, BufferUsage usage, const void *data = nullptr);

    template <typename Type>
    HardwareBuffer(const Type &input, BufferUsage usage)
        requires is_container<Type>
        : HardwareBuffer(input.size() * sizeof(input[0]), usage, input.data())
    {
    }

    ~HardwareBuffer();

    uint32_t storeDescriptor();

    bool copyFromBuffer(const HardwareBuffer &inputBuffer, HardwareExecutor *executor);
    bool copyFromData(const void *inputData, uint64_t size);

    template <typename Type>
    bool copyFromVector(const std::vector<Type> &input)
    {
        copyFromData(input.data(), input.size() * sizeof(Type));
        return true;
    }

    void* getMappedData();

    uint64_t getBufferSize();

    HardwareBuffer &operator=(const HardwareBuffer &other);

    operator bool();

    std::shared_ptr<uintptr_t> bufferID;
};

struct HardwareImage
{
    HardwareImage();
    HardwareImage(const HardwareImage &other);
    HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage = ImageUsage::SampledImage, int arrayLayers = 1, void *imageData = nullptr);

    ~HardwareImage();

    operator bool();

    uint32_t storeDescriptor();

    //bool blitFromImage(const HardwareImage &buffer);
    //bool copyFromImage(const HardwareImage &buffer);

    HardwareImage &operator=(const HardwareImage &other);

    std::shared_ptr<uintptr_t> imageID;

  private:
    HardwareImage &copyFromBuffer(const HardwareBuffer &buffer);
    HardwareImage &copyFromData(const void *inputData);
};

struct HardwarePushConstant
{
    HardwarePushConstant();
    ~HardwarePushConstant();

    HardwarePushConstant(const HardwarePushConstant &other);

    template <typename Type>
    HardwarePushConstant(Type data)
        requires(!std::is_same_v<std::remove_cvref_t<Type>, HardwarePushConstant>)
    {
        copyFromRaw(&data, sizeof(Type));
    }

    HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant *whole = nullptr);

    HardwarePushConstant &operator=(const HardwarePushConstant &other);

    uint8_t *getData() const;
    uint64_t getSize() const;

  private:
    void copyFromRaw(const void *src, uint64_t size);

    std::shared_ptr<uintptr_t> pushConstantID;
};

struct HardwareDisplayer
{
    HardwareDisplayer(void *surface = nullptr);
    HardwareDisplayer(const HardwareDisplayer &other);
    ~HardwareDisplayer();

    HardwareDisplayer &operator=(const HardwareImage &image);
    HardwareDisplayer &operator=(const HardwareDisplayer &other);
    //void setSurface(void *surface);

    std::shared_ptr<uintptr_t> displaySurfaceID;

    //void *displaySurface = nullptr;
};
