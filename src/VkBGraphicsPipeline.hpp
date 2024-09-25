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
  static std::vector<char> readShader(const std::string& filename);
  static VkShaderModule createShaderModule(const std::vector<char>& code);
  void createGraphicsPipeline(VkBSwapChain& swapChain,
			      VkBRenderPass renderPass,
			      VkDescriptorSetLayout* descriptorSetLayouts);
};
