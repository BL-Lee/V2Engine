#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"


class VkBRenderPass
{
public:
  VkRenderPass renderPass; 
  void createRenderPass(VkDevice device, VkFormat format);
};
