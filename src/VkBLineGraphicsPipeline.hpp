#include "VkBGlobals.hpp"
#include <iostream>
#include <vector>
#include "swapChain.hpp"
#include "VkBRenderPass.hpp"
class VkBLineGraphicsPipeline {
public:
  VkPipeline pipeline;
  VkPipelineLayout layout;

  
  void createLineGraphicsPipeline(VkBSwapChain& swapChain,
			      VkBRenderPass renderPass,
				  std::vector<VkDescriptorSetLayout>* descriptorSetLayouts,
				  std::vector<VkPushConstantRange>* pushConstantRanges);

};
