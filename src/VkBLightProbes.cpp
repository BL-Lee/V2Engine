#include "VkBLightProbes.hpp"
#include "VkBBuffer.hpp"
#include "VkBSingleCommandBuffer.hpp"
#include <stdexcept>
#include <iostream>
void VkBLightProbeInfo::create(int frameCount)
  {
    resolution = 32;

    cascadeCount = 4;
    
    gridDimensions = glm::vec3(2.2f,2.2f,2.2f);//Doesn't do anything yet in the shader. Is defined in the shader
    center = glm::vec3(0.0f,0.0f,0.0f);//Doesn't do anything yet in the shader
    raysPerProbe = 20; //Same

    imageWidth = resolution * 2;

    semaphoreChain = (VkSemaphore*)calloc(cascadeCount, sizeof(VkSemaphore));
    for(int i = 0; i < cascadeCount; i++)
      {
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphoreChain[i]) != VK_SUCCESS)
	  throw std::runtime_error("failed to create semaphores!");
      }
    
    debugCascadeViewIndex = 0;
    debugDirectionViewIndex = 0;
    viewDebug = 0;
    lineCount = imageWidth * imageWidth * imageWidth;
    lines = (LineVertex*)calloc(2 * lineCount, sizeof(LineVertex));
    lineVBO.create(lineCount * 2 * sizeof(LineVertex));

    computeUniformPool.create(cascadeCount, frameCount, 0, true);
    computeUniformPool.addImage(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    computeUniformPool.addImage(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    computeUniformPool.addStorageBuffer(1, lineCount * 2 * sizeof(LineVertex));
    computeUniformPool.createDescriptorSetLayout();

    textures = (VkBTexture*)calloc(cascadeCount + 1, sizeof(VkBTexture));
    for (int i = 0; i < cascadeCount; i++)
      {
	textures[i].createTextureImage3D(VKB_TEXTURE_TYPE_STORAGE_SAMPLED_RGBA,
						   resolution * 2,
						   resolution * 2,
						   resolution * 2,
						   nullptr);
      }

    //Idk how to bind an empty image, so make a 1x1x1 temp image for the top cascade
    //2x2x2 because 1x1x1 kinda implies 2d (depth=1)
    uint8_t test_pix[2*2*2*4] = {1,0,0,0,
      0,1,0,0,
      1,1,0,0,
      0,0,1,0,
      1,0,1,0,
      0,1,1,0,
      1,1,1,0,
      0,0,0,0
    };
    textures[cascadeCount].createTextureImage3D(VKB_TEXTURE_TYPE_SAMPLED_RGBA,
    						   2,
    						   2,
    						   2,
    						   test_pix);
    std::cout << "buh"<< std::endl;
    computeUniforms = (VkBUniformBuffer*)calloc(cascadeCount, sizeof(VkBUniformBuffer));
    for (int i = cascadeCount - 1; i >= 0; i--)
      {
	VkImageView probeViews[2];
	VkSampler probeSamplers[2];
	
	probeViews[0] = textures[i].imageView;
	probeSamplers[0] = textures[i].textureSampler;
	probeViews[1] = textures[i + 1].imageView;
	probeSamplers[1] = textures[i + 1].textureSampler;

	computeUniforms[i].allocateDescriptorSets(&computeUniformPool, probeViews, probeSamplers, &lineVBO.vertexBuffer);
      }
    

    drawUniformPool.create(1, frameCount, 0, true);
    drawUniformPool.addImageArray(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				  cascadeCount);
    drawUniformPool.addStorageBuffer(1, lineCount * 2 * sizeof(LineVertex));
    drawUniformPool.createDescriptorSetLayout();

    std::vector<VkImageView> probeViews;
    std::vector<VkSampler> probeSamplers;
    probeViews.resize(cascadeCount);
    probeSamplers.resize(cascadeCount);

    for (int i = 0; i < cascadeCount; i++)
      {
	probeViews[i] = textures[i].imageView;
	probeSamplers[i] = textures[i].textureSampler;
      }
    drawUniform.allocateDescriptorSets(&drawUniformPool, probeViews.data(), probeSamplers.data(), &lineVBO.vertexBuffer);
    
  }


void VkBLightProbeInfo::transitionImagesToStorage()
{
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  std::vector<VkImageMemoryBarrier> barriers; //TODO: Unhardcode
  barriers.resize(cascadeCount);
  for (int i = 0; i < cascadeCount; i++)
    {
      barriers[i] = {0};
      barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
      barriers[i].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      barriers[i].newLayout = VK_IMAGE_LAYOUT_GENERAL;
      barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
      barriers[i].image = textures[i].image;
      barriers[i].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      barriers[i].subresourceRange.baseMipLevel = 0; //no mips and not an array 
      barriers[i].subresourceRange.levelCount = 1;
      barriers[i].subresourceRange.baseArrayLayer = 0;
      barriers[i].subresourceRange.layerCount = 1;
    

      barriers[i].srcAccessMask = 0;
      barriers[i].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    }
  VkPipelineStageFlags sourceStage =  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;


  vkCmdPipelineBarrier(
		       commandBuffer,
		       sourceStage, destinationStage,
		       0,
		       0, nullptr,
		       0, nullptr,
		       cascadeCount, &barriers[0]
		       );

    
  vKEndSingleTimeCommandBuffer(commandBuffer);
}
void VkBLightProbeInfo::transitionImageToSampled(int cascadeIndex)
{
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = textures[cascadeIndex].image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;


  barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (cascadeIndex == 0) //Last one, will switch to draw pipeline after this
    {
      sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
      destinationStage =  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
  else
    {
      sourceStage =  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
      destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }


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

void VkBLightProbeInfo::destroy() {
  for (int i = 0; i < cascadeCount; i++)
    {
    textures[i].destroy();
    vkDestroySemaphore(device, semaphoreChain[i], nullptr);
    }
  textures[cascadeCount].destroy();
  computeUniformPool.destroy();
  drawUniformPool.destroy();
  free(textures);
  free(semaphoreChain);
  free(computeUniforms);
  
  lineVBO.destroy();
  free(lines);

}
