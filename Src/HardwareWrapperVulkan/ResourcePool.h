#pragma once

#include "HardwareWrapperVulkan/HardwareVulkan/ResourceManager.h"
#include "corona/kernel/utils/storage.h"


extern Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap> globalBufferStorages;
extern Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap> globalImageStorages;


[[nodiscard]] inline ResourceManager::BufferHardwareWrap getBufferFromHandle(uintptr_t handle) {
    ResourceManager::BufferHardwareWrap buffer;
    auto read_handle = globalBufferStorages.acquire_read(handle);
    buffer = *read_handle;
    return buffer;
}

[[nodiscard]] inline ResourceManager::ImageHardwareWrap getImageFromHandle(uintptr_t handle) {
    ResourceManager::ImageHardwareWrap image;
    auto read_handle = globalImageStorages.acquire_read(handle);
    image = *read_handle;
    return image;
}