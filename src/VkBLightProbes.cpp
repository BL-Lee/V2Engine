#include "VkBLightProbes.hpp"
#include "VkBSingleCommandBuffer.hpp"
void VkBLightProbeInfo::create()
  {
    resolution = 32;

    cascadeCount = 2;
    
    
    gridDimensions = glm::vec3(2.0f,2.0f,2.0f);//Doesn't do anything yet. Is defined in the shader
    center = glm::vec3(0.0f,0.0f,0.0f);//Doesn't do anything yet
    raysPerProbe = 20; //Same
  }

void VkBLightProbeInfo::transitionImageToSampled(VkImage image)
{
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
    
  VkPipelineStageFlags sourceStage =  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

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
void VkBLightProbeInfo::transitionImageToStorage(VkImage image)
{
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
    
  VkPipelineStageFlags sourceStage =  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
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

void VkBLightProbeInfo::copyTextureToCPU(VkBTexture tex)
{
  
}
void VkBLightProbeInfo::destroy() {
}
