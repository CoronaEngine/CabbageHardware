#include "ResourceCommand.h"
#include <Hardware/GlobalContext.h>

// 小工具：插入一次通用的内存屏障，确保写入对后续阶段可见
static inline void InsertMemoryBarrier(
	VkCommandBuffer cmd,
	VkPipelineStageFlags srcStage,
	VkAccessFlags srcAccess,
	VkPipelineStageFlags dstStage,
	VkAccessFlags dstAccess)
{
	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;
	vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void CopyBufferCommand::commitCommand(HardwareExecutor &hardwareExecutor)
{
	VkCommandBuffer cmd = hardwareExecutor.currentRecordQueue->commandBuffer;

	// 执行拷贝
	srcBuffer.resourceManager->copyBuffer(cmd, srcBuffer, dstBuffer);

	// 在拷贝后加入屏障：让 TRANSFER 写入对后续任何阶段的读/写可见
	InsertMemoryBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT);
}

void CopyImageCommand::commitCommand(HardwareExecutor &hardwareExecutor)
{
	VkCommandBuffer cmd = hardwareExecutor.currentRecordQueue->commandBuffer;

	// 确保布局满足拷贝需求（此处统一使用 GENERAL，便于跨用途）
	srcImage.resourceManager->transitionImageLayout(
		cmd, srcImage,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT);

	dstImage.resourceManager->transitionImageLayout(
		cmd, dstImage,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT);

	// 执行拷贝
	srcImage.resourceManager->copyImage(cmd, srcImage, dstImage);

	// 拷贝后加入屏障，保证写入可见
	InsertMemoryBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT);
}

void CopyBufferToImageCommand::commitCommand(HardwareExecutor &hardwareExecutor)
{
	VkCommandBuffer cmd = hardwareExecutor.currentRecordQueue->commandBuffer;

	// 先保证之前对 buffer 的写入对传输读可见
	InsertMemoryBarrier(
		cmd,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_MEMORY_WRITE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_READ_BIT);

	// 目标 image 过渡到 TRANSFER_DST_OPTIMAL
	dstImage.resourceManager->transitionImageLayout(
		cmd, dstImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT);

	// 执行拷贝
	dstImage.resourceManager->copyBufferToImage(cmd, srcBuffer, dstImage);

	// 拷贝后确保写入对后续阶段可见（不改变布局，后续根据用途再过渡）
	InsertMemoryBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT);
}

void CopyImageToBufferCommand::commitCommand(HardwareExecutor &hardwareExecutor)
{
	VkCommandBuffer cmd = hardwareExecutor.currentRecordQueue->commandBuffer;

	// 先保证之前对 image 的写入对传输读可见
	InsertMemoryBarrier(
		cmd,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_MEMORY_WRITE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_READ_BIT);

	// 源 image 过渡到 TRANSFER_SRC_OPTIMAL
	srcImage.resourceManager->transitionImageLayout(
		cmd, srcImage,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT);

	// 执行拷贝
	srcImage.resourceManager->copyImageToBuffer(cmd, srcImage, dstBuffer);

	// 拷贝后确保写入对后续阶段可见
	InsertMemoryBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT);
}

void BlitImageCommand::commitCommand(HardwareExecutor &hardwareExecutor)
{
	VkCommandBuffer cmd = hardwareExecutor.currentRecordQueue->commandBuffer;

	// 过渡布局以进行 Blit
	srcImage.resourceManager->transitionImageLayout(
		cmd, srcImage,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT);

	dstImage.resourceManager->transitionImageLayout(
		cmd, dstImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT);

	// 使用 blit（如果已有实现可替换为 resourceManager.blitImage）
	// 这里直接用 vkCmdBlitImage 更通用（需要 COLOR aspect）
	VkImageBlit imageBlit{};
	imageBlit.srcSubresource.aspectMask = srcImage.aspectMask;
	imageBlit.srcSubresource.mipLevel = 0;
	imageBlit.srcSubresource.baseArrayLayer = 0;
	imageBlit.srcSubresource.layerCount = 1;
	imageBlit.srcOffsets[0] = {0, 0, 0};
	imageBlit.srcOffsets[1] = {static_cast<int32_t>(srcImage.imageSize.x), static_cast<int32_t>(srcImage.imageSize.y), 1};

	imageBlit.dstSubresource.aspectMask = dstImage.aspectMask;
	imageBlit.dstSubresource.mipLevel = 0;
	imageBlit.dstSubresource.baseArrayLayer = 0;
	imageBlit.dstSubresource.layerCount = 1;
	imageBlit.dstOffsets[0] = {0, 0, 0};
	imageBlit.dstOffsets[1] = {static_cast<int32_t>(dstImage.imageSize.x), static_cast<int32_t>(dstImage.imageSize.y), 1};

	vkCmdBlitImage(cmd,
				   srcImage.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				   dstImage.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				   1, &imageBlit, VK_FILTER_LINEAR);

	// 写入可见屏障
	InsertMemoryBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT);
}

