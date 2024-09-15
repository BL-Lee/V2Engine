#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VkBCommandBuffer{

public:
  VkCommandBuffer commandBuffer;
  
  void createCommandBuffer(VkDevice device, VkCommandPool commandPool);
  
  void record(VkRenderPass renderPass,
	      VkFramebuffer frameBuffer,
	      VkExtent2D extent,
	      VkPipeline pipeline);
};
