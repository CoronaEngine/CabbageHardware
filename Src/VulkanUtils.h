#pragma once

#include <cstdio>
#include <cstdint>
#include "Hardware/GlobalContext.h"

#if __has_include(<source_location>)
#include <source_location>
#define CH_USE_SOURCE_LOCATION 1
#else
#define CH_USE_SOURCE_LOCATION 0
#endif

#ifdef CABBAGE_ENGINE_DEBUG
// Forward declaration to allow use in vkCheck before full definition below.
static inline const char *vkResultStr(VkResult ret);
#if CH_USE_SOURCE_LOCATION
static inline VkResult vkCheck(VkResult result, const std::source_location &loc = std::source_location::current())
{
    if (result != VK_SUCCESS)
    {
        std::fprintf(stderr, "VkResult: %s, %s:%d, %s\n", vkResultStr(result), loc.file_name(), static_cast<int>(loc.line()), loc.function_name());
        abort();
    }
    return result;
}
#else
// Fallback without std::source_location support. File/line are omitted.
static inline VkResult vkCheck(VkResult result)
{
    if (result != VK_SUCCESS)
    {
        std::fprintf(stderr, "VkResult: %s\n", vkResultStr(result));
        abort();
    }
    return result;
}
#endif
#else
// Release build: keep behavior as a lightweight pass-through.
static inline VkResult vkCheck(VkResult result)
{
    return result;
}
#endif

static inline const char *vkResultStr(VkResult ret)
{
  switch ((VkResult) ret) {
    case VK_ERROR_INITIALIZATION_FAILED:
      return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_NOT_PERMITTED_EXT:
      return "VK_ERROR_NOT_PERMITTED_EXT";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_NOT_READY:
      return "VK_NOT_READY";
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_TIMEOUT:
      return "VK_TIMEOUT";
    case VK_ERROR_FRAGMENTED_POOL:
      return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_FRAGMENTATION_EXT:
      return "VK_ERROR_FRAGMENTATION_EXT";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_SUCCESS:
      return "VK_SUCCESS";
    case VK_ERROR_INVALID_SHADER_NV:
      return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_SURFACE_LOST_KHR:
      return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
      return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_SUBOPTIMAL_KHR:
      return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_TOO_MANY_OBJECTS:
      return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_EVENT_RESET:
      return "VK_EVENT_RESET";
    case VK_ERROR_OUT_OF_DATE_KHR:
      return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_ERROR_MEMORY_MAP_FAILED:
      return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_EVENT_SET:
      return "VK_EVENT_SET";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_INCOMPLETE:
      return "VK_INCOMPLETE";
    case VK_ERROR_DEVICE_LOST:
      return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "VK_ERROR_OUT_OF_HOST_MEMORY";
    default:
      return "Unhandled VkResult";
  }
}