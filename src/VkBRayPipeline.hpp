#include "VkBGlobals.hpp"
#include "VkBUniformPool.hpp"
#include "VkBUniformBuffer.hpp"
#include "VkBTexture.hpp"
#include <vector>
class VkBRayPipeline
{
public:
  
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  VkCommandBuffer commandBuffer;

  void destroy();
  void createPipeline(const char* filePath,
		      std::vector<VkDescriptorSetLayout>* descriptorSetLayouts,
		      std::vector<VkPushConstantRange>* pushConstantsRanges);
  //Shader stuff
  void transitionSwapChainForComputeWrite(VkImage image, VkImage swapImage);
  void transitionSwapChainForComputeTransfer(VkImage image);
  void transitionSwapChainForComputePresent(VkImage swapImage);
  void copyTextureToSwapChain(VkImage swapChainImage, VkImage sourceImage,
			      uint32_t width, uint32_t height);

};
