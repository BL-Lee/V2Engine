#include "VkBComputePipeline.hpp"
#include "VkBShader.hpp"
#include "VkBSingleCommandBuffer.hpp"
void VkBComputePipeline::createPipeline(const char* filePath,
		      std::vector<VkDescriptorSetLayout>* descriptorSetLayouts,
		      std::vector<VkPushConstantRange>* pushConstantRanges,
		      int commandBufferCount)
{
  //Shader stuff
  VkShaderModule computeShaderModule = VkBShader::createShaderFromFile(filePath);

  VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
  computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  computeShaderStageInfo.module = computeShaderModule;
  computeShaderStageInfo.pName = "main";

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  if (descriptorSetLayouts)
    {
      pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts->size();
      pipelineLayoutInfo.pSetLayouts = (VkDescriptorSetLayout*)descriptorSetLayouts->data();
    }
  else
    {
      pipelineLayoutInfo.setLayoutCount = 0;
      pipelineLayoutInfo.pSetLayouts = nullptr;
    }
  if (pushConstantRanges)
    {
      pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges->size(); // Optional
      pipelineLayoutInfo.pPushConstantRanges = (VkPushConstantRange*)pushConstantRanges->data();
    }
  else
    {
      pipelineLayoutInfo.pushConstantRangeCount = 0;
      pipelineLayoutInfo.pPushConstantRanges = nullptr;
    }

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create compute pipeline layout!");
  }
    
  VkComputePipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.stage = computeShaderStageInfo;

  if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create compute pipeline!");
  }

  vkDestroyShaderModule(device, computeShaderModule, nullptr);


  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = computeCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = commandBufferCount;
  VkCommandBuffer* buffers;
  if (commandBufferCount == 1)
    {
      buffers = &commandBuffer;
      commandBuffers = nullptr;
    }
  else
    {
      commandBuffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * commandBufferCount);
      buffers = commandBuffers;
    }
  if (vkAllocateCommandBuffers(device, &allocInfo, buffers) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }

}

void VkBComputePipeline::createPipeline(const char* filePath,
				    std::vector<VkDescriptorSetLayout>* descriptorSetLayouts,
				    std::vector<VkPushConstantRange>* pushConstantRanges) {
  createPipeline(filePath, descriptorSetLayouts, pushConstantRanges,1);
}

void VkBComputePipeline::transitionSampledImageForComputeWrite(VkImage image) {

  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
    
  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(
		       commandBuffer,
		       sourceStage, destinationStage,
		       0,
		       0, nullptr,
		       0, nullptr,
		       1, &barrier
		       );
  vKEndSingleTimeCommandBuffer(commandBuffer);

}
void VkBComputePipeline::transitionImageForComputeSample(VkImage image) {
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
    
  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  //  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(
		       commandBuffer,
		       sourceStage, destinationStage,
		       0,
		       0, nullptr,
		       0, nullptr,
		       1, &barrier
		       );
  vKEndSingleTimeCommandBuffer(commandBuffer);

}

void VkBComputePipeline::transitionSwapChainForComputeWrite(VkImage image, VkImage swapImage) {

  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
    
  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(
		       commandBuffer,
		       sourceStage, destinationStage,
		       0,
		       0, nullptr,
		       0, nullptr,
		       1, &barrier
		       );

    
  VkImageMemoryBarrier swapBarrier{};
  swapBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  swapBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  swapBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  swapBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  swapBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  swapBarrier.image = swapImage;
  swapBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  swapBarrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  swapBarrier.subresourceRange.levelCount = 1;
  swapBarrier.subresourceRange.baseArrayLayer = 0;
  swapBarrier.subresourceRange.layerCount = 1;

  swapBarrier.srcAccessMask = 0;
  swapBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
  vkCmdPipelineBarrier(
		       commandBuffer,
		       sourceStage, destinationStage,
		       0,
		       0, nullptr,
		       0, nullptr,
		       1, &swapBarrier
		       );

  vKEndSingleTimeCommandBuffer(commandBuffer);


}
void VkBComputePipeline::transitionImageForComputeTransfer(VkImage image) {

  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
    
  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

  barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.dstAccessMask = 0;

  vkCmdPipelineBarrier(
		       commandBuffer,
		       sourceStage, destinationStage,
		       0,
		       0, nullptr,
		       0, nullptr,
		       1, &barrier
		       );
  vKEndSingleTimeCommandBuffer(commandBuffer);


}
  
void VkBComputePipeline::transitionImageForComputePresent(VkImage swapImage)
{
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier swapBarrier{};
  swapBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  swapBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  swapBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  swapBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  swapBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  swapBarrier.image = swapImage;
  swapBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  swapBarrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  swapBarrier.subresourceRange.levelCount = 1;
  swapBarrier.subresourceRange.baseArrayLayer = 0;
  swapBarrier.subresourceRange.layerCount = 1;
    
  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

  swapBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  swapBarrier.dstAccessMask = 0;

    
  vkCmdPipelineBarrier(
		       commandBuffer,
		       sourceStage, destinationStage,
		       0,
		       0, nullptr,
		       0, nullptr,
		       1, &swapBarrier
		       );

  vKEndSingleTimeCommandBuffer(commandBuffer);
    
}


void VkBComputePipeline::copyTextureToSwapChain(VkImage swapChainImage,
					    VkImage sourceImage,
			    uint32_t width, uint32_t height)
{
  VkCommandBuffer transferCommandBuffer = vKBeginSingleTimeCommandBuffer();

  VkImageCopy imageCopyInfo {};
  VkExtent3D extent{};
  extent.width = width;
  extent.height = height;
  extent.depth = 1;
  imageCopyInfo.extent = extent;
  VkImageSubresourceLayers resourceLayerInfo{};
  resourceLayerInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  resourceLayerInfo.mipLevel = 0;
  resourceLayerInfo.baseArrayLayer = 0;
  resourceLayerInfo.layerCount = 1;
  VkOffset3D offset{};
  offset.x = 0; offset.y = 0; offset.z = 0;
  imageCopyInfo.srcSubresource = resourceLayerInfo;
  imageCopyInfo.srcOffset = offset;
  imageCopyInfo.dstSubresource = resourceLayerInfo;
  imageCopyInfo.dstOffset = offset;
  vkCmdCopyImage(
		 transferCommandBuffer,
		 sourceImage,
		 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		 swapChainImage,
		 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		 1,
		 &imageCopyInfo
		 );
	
  vKEndSingleTimeCommandBuffer(transferCommandBuffer);

}
void VkBComputePipeline::destroy()
{
  vkDestroyPipeline(device, pipeline, nullptr);	
  vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
