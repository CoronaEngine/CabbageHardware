#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"

/// Per-resource access descriptor collected during setResource / setResourceDirect.
/// Keyed by push-constant byte-offset so that re-binding the same slot
/// replaces the old entry (persistent across frames, no manual clear needed).
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
/// Entries are keyed by push-constant byte-offset, so re-setting a slot replaces
/// the previous resource.  The map persists across frames: resources set once
/// remain tracked until overwritten — matching the persistent push-constant semantics.
struct ResourceAccessTracker
{
    /// Record a buffer access bound at the given push-constant byte offset.
    void trackBuffer(uint64_t byteOffset,
                     uintptr_t bufferID,
                     VkPipelineStageFlags2 stageMask,
                     VkAccessFlags2 accessMask)
    {
        accesses_[byteOffset] = ResourceAccess{
            .resourceID = bufferID,
            .isImage = false,
            .dstStageMask = stageMask,
            .dstAccessMask = accessMask,
            .requiredLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
    }

    /// Record an image access bound at the given push-constant byte offset.
    void trackImage(uint64_t byteOffset,
                    uintptr_t imageID,
                    VkPipelineStageFlags2 stageMask,
                    VkAccessFlags2 accessMask,
                    VkImageLayout requiredLayout)
    {
        accesses_[byteOffset] = ResourceAccess{
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

    /// Key = push-constant byte offset where the resource handle is stored.
    std::unordered_map<uint64_t, ResourceAccess> accesses_;
};
