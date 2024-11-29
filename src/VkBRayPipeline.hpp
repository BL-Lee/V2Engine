#pragma once
#include "VkBGlobals.hpp"
#include "VkBUniformPool.hpp"
#include "VkBUniformBuffer.hpp"
#include "VkBTexture.hpp"
#include <vector>

struct RayDebugPushConstant
{
  uint32_t viewMode;
  float triangleTestLimit;
  float boxTestLimit;
};


class VkBRayPipeline
{
public:
  
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  VkCommandBuffer commandBuffer;
  //kind of ahack so we can have radiance cascades that wait for the previous in the chain
  VkCommandBuffer* commandBuffers;

  
  void destroy();
  void createPipeline(const char* filePath,
		      std::vector<VkDescriptorSetLayout>* descriptorSetLayouts,
		      std::vector<VkPushConstantRange>* pushConstantsRanges);
  void createPipeline(const char* filePath,
		      std::vector<VkDescriptorSetLayout>* descriptorSetLayouts,
		      std::vector<VkPushConstantRange>* pushConstantsRanges,
		      int commandBufferCount);

  //Shader stuff
  void transitionSwapChainForComputeWrite(VkImage image, VkImage swapImage);
  void transitionSwapChainForComputeTransfer(VkImage image);
  void transitionSwapChainForComputePresent(VkImage swapImage);
  void copyTextureToSwapChain(VkImage swapChainImage, VkImage sourceImage,
			      uint32_t width, uint32_t height);

};
