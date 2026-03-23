#include "ResourceAccessTracker.h"

#include "HardwareWrapperVulkan/ResourcePool.h"

CommandRecordVulkan::RequiredBarriers ResourceAccessTracker::buildBarriers(
    const std::unordered_set<uintptr_t> &skipImageIDs) const
{
    CommandRecordVulkan::RequiredBarriers barriers;

    for (const auto &[resourceID, access] : accesses_)
    {
        // Skip resources that already have dedicated barriers
        if (access.isImage && skipImageIDs.contains(access.resourceID))
            continue;

        if (access.isImage)
        {
            {
                auto handle = globalImageStorages.acquire_read(access.resourceID);
                if (handle->imageHandle == VK_NULL_HANDLE)
                    continue;

                VkImageMemoryBarrier2 imageBarrier{};
                imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                imageBarrier.pNext = nullptr;
                imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
                imageBarrier.dstStageMask = access.dstStageMask;
                imageBarrier.dstAccessMask = access.dstAccessMask;
                imageBarrier.oldLayout = handle->imageLayout;
                imageBarrier.newLayout = access.requiredLayout;
                imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageBarrier.image = handle->imageHandle;
                imageBarrier.subresourceRange.aspectMask = handle->aspectMask;
                imageBarrier.subresourceRange.baseMipLevel = 0;
                imageBarrier.subresourceRange.levelCount = handle->mipLevels;
                imageBarrier.subresourceRange.baseArrayLayer = 0;
                imageBarrier.subresourceRange.layerCount = handle->arrayLayers;

                barriers.imageBarriers.push_back(imageBarrier);
            }

            // Update layout after barrier (release read lock first via scope)
            {
                auto writeHandle = globalImageStorages.acquire_write(access.resourceID);
                writeHandle->imageLayout = access.requiredLayout;
            }
        }
        else
        {
            auto handle = globalBufferStorages.acquire_read(access.resourceID);
            if (handle->bufferHandle == VK_NULL_HANDLE)
                continue;

            VkBufferMemoryBarrier2 bufferBarrier{};
            bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            bufferBarrier.pNext = nullptr;
            bufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            bufferBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
            bufferBarrier.dstStageMask = access.dstStageMask;
            bufferBarrier.dstAccessMask = access.dstAccessMask;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.buffer = handle->bufferHandle;
            bufferBarrier.offset = 0;
            bufferBarrier.size = VK_WHOLE_SIZE;

            barriers.bufferBarriers.push_back(bufferBarrier);
        }
    }

    return barriers;
}
