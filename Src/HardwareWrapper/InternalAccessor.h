#pragma once

// Internal helper file for HardwareWrapper implementations
// This file should NOT be included in public headers like CabbageHardware.h

#include <cstdint>

// Forward declarations
struct HardwareExecutorVulkan;

// Internal accessor functions to get Vulkan implementations from wrapper IDs
// These functions are implemented in their respective wrapper .cpp files

// Get HardwareExecutorVulkan implementation from executor ID
HardwareExecutorVulkan* getExecutorImpl(uintptr_t id);
