#include "CabbageHardware.h"
#include "HardwareCommands.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

// ================= CopyCommandImpl 基类 =================
struct CopyCommandImpl {
    virtual ~CopyCommandImpl() = default;
    virtual CommandRecordVulkan* getCommandRecord() = 0;
};

// ================= Buffer 到 Buffer 拷贝命令实现 =================
struct BufferCopyCommandImpl : CopyCommandImpl {
    uint64_t srcBufferID{0};
    uint64_t dstBufferID{0};
    uint64_t srcOffset{0};
    uint64_t dstOffset{0};
    uint64_t size{0};

    std::unique_ptr<CopyBufferCommand> command;

    CommandRecordVulkan* getCommandRecord() override {
        if (srcBufferID == 0 || dstBufferID == 0) {
            return nullptr;
        }

        // 按 ID 顺序获取锁以避免死锁
        if (srcBufferID < dstBufferID) {
            auto srcHandle = globalBufferStorages.acquire_write(srcBufferID);
            auto dstHandle = globalBufferStorages.acquire_write(dstBufferID);
            command = std::make_unique<CopyBufferCommand>(*srcHandle, *dstHandle);
        } else {
            auto dstHandle = globalBufferStorages.acquire_write(dstBufferID);
            auto srcHandle = globalBufferStorages.acquire_write(srcBufferID);
            command = std::make_unique<CopyBufferCommand>(*srcHandle, *dstHandle);
        }

        return command.get();
    }
};

// ================= Buffer 到 Image 拷贝命令实现 =================
struct BufferToImageCommandImpl : CopyCommandImpl {
    uint64_t srcBufferID{0};
    uint64_t dstImageID{0};
    uint64_t bufferOffset{0};
    uint32_t imageLayer{0};
    uint32_t imageMip{0};

    std::unique_ptr<CopyBufferToImageCommand> command;

    CommandRecordVulkan* getCommandRecord() override {
        if (srcBufferID == 0 || dstImageID == 0) {
            return nullptr;
        }

        auto srcHandle = globalBufferStorages.acquire_write(srcBufferID);
        auto dstHandle = globalImageStorages.acquire_write(dstImageID);
        command = std::make_unique<CopyBufferToImageCommand>(*srcHandle, *dstHandle, imageMip);

        return command.get();
    }
};

// ================= BufferCopyCommand 构造函数 =================
BufferCopyCommand::BufferCopyCommand(const HardwareBuffer& src, const HardwareBuffer& dst,
                                     uint64_t srcOffset, uint64_t dstOffset, uint64_t size) {
    auto implPtr = std::make_shared<BufferCopyCommandImpl>();
    implPtr->srcBufferID = src.getBufferID();
    implPtr->dstBufferID = dst.getBufferID();
    implPtr->srcOffset = srcOffset;
    implPtr->dstOffset = dstOffset;
    implPtr->size = size;
    impl = implPtr;
}

// ================= BufferToImageCommand 构造函数 =================
BufferToImageCommand::BufferToImageCommand(const HardwareBuffer& src, const HardwareImage& dst,
                                           uint64_t bufferOffset, uint32_t imageLayer, uint32_t imageMip) {
    auto implPtr = std::make_shared<BufferToImageCommandImpl>();
    implPtr->srcBufferID = src.getBufferID();
    implPtr->dstImageID = dst.getImageID();
    implPtr->bufferOffset = bufferOffset;
    implPtr->imageLayer = imageLayer;
    implPtr->imageMip = imageMip;
    impl = implPtr;
}
