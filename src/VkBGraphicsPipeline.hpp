#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <iostream>
#include <vector>
#include "swapChain.hpp"
#include "VkBRenderPass.hpp"

class VkBGraphicsPipeline {
public:
  VkPipeline pipeline;

  //So I think
  /*
    
    Pipeline -> what to do with the inputted buffers and triangles
    Render Pass -> where to put them (attachments)

    Maybe its easier to think of like
    A pipeline as a set of shaders.
    Different shaders -> different pipeline

    So you can use multiple pipelines (Shader sets) during a single render pass?

  */
  VkPipelineLayout layout;
  bool isLineMode;
  void createGraphicsPipeline(VkBSwapChain& swapChain,
			      VkBRenderPass renderPass,
			      const char* vertFile,
			      const char* fragFile,
			      std::vector<VkDescriptorSetLayout>* descriptorSetLayouts,
			      std::vector<VkPushConstantRange>* pushConstantRanges);
  void createLinePipeline(VkBSwapChain& swapChain,
			      VkBRenderPass renderPass,
			      const char* vertFile,
			      const char* fragFile,
			      std::vector<VkDescriptorSetLayout>* descriptorSetLayouts,
			      std::vector<VkPushConstantRange>* pushConstantRanges);
  void createPipeline(VkBSwapChain& swapChain,
		      VkBRenderPass renderPass,
		      const char* vertFile,
		      const char* fragFile,
		      std::vector<VkDescriptorSetLayout>* descriptorSetLayouts,
		      std::vector<VkPushConstantRange>* pushConstantRanges);

  void destroy();
};
