#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"

/// Per-resource access descriptor collected during setResource / setResourceDirect.
struct ResourceAccess
{
    uintptr_t resourceID{0};
    bool isImage{false};

    VkPipelineStageFlags2 dstStageMask{0};
    VkAccessFlags2 dstAccessMask{0};

    /// Target layout for images; VK_IMAGE_LAYOUT_UNDEFINED for buffers.
    VkImageLayout requiredLayout{VK_IMAGE_LAYOUT_UNDEFINED};
};

/// Holds the set of bindless resources a pipeline accesses via push-constant handles.
///
/// Keyed by **resourceID** so that N different resources bound at the same
/// push-constant byte-offset (across multiple record() calls) each get their
/// own barrier.  Re-binding the same resource (same ID) is automatically
/// deduplicated.
struct ResourceAccessTracker
{
    /// Record a buffer access.  byteOffset is kept for API compatibility but
    /// the map key is bufferID so every unique buffer is tracked.
    void trackBuffer(uint64_t /*byteOffset*/,
                     uintptr_t bufferID,
                     VkPipelineStageFlags2 stageMask,
                     VkAccessFlags2 accessMask)
    {
        accesses_[bufferID] = ResourceAccess{
            .resourceID = bufferID,
            .isImage = false,
            .dstStageMask = stageMask,
            .dstAccessMask = accessMask,
            .requiredLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
    }

    /// Record an image access.  byteOffset is kept for API compatibility but
    /// the map key is imageID so every unique image is tracked.
    void trackImage(uint64_t /*byteOffset*/,
                    uintptr_t imageID,
                    VkPipelineStageFlags2 stageMask,
                    VkAccessFlags2 accessMask,
                    VkImageLayout requiredLayout)
    {
        accesses_[imageID] = ResourceAccess{
            .resourceID = imageID,
            .isImage = true,
            .dstStageMask = stageMask,
            .dstAccessMask = accessMask,
            .requiredLayout = requiredLayout,
        };
    }

    /// Build precise per-resource barriers from the tracked accesses.
    /// @param skipImageIDs  Set of image IDs that already have dedicated barriers
    ///                      (e.g. render targets, depth), to avoid duplicates.
    /// Implementation in ResourceAccessTracker.cpp (avoids circular include with ResourcePool.h).
    CommandRecordVulkan::RequiredBarriers buildBarriers(
        const std::unordered_set<uintptr_t> &skipImageIDs = {}) const;

    [[nodiscard]] bool empty() const { return accesses_.empty(); }
    void clear() { accesses_.clear(); }

    /// Key = resourceID (uintptr_t).  One entry per unique resource.
    std::unordered_map<uintptr_t, ResourceAccess> accesses_;
};
