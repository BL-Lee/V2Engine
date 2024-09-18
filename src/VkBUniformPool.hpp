#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vector>
class VkBUniformPool {

public:
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  //Do it in this order
  //[frame_1_UBO_1, frame_2_UBO_1,... frame_1_UBO_2, frame_2_UBO_2, 
  int imagesInFlight;
  int totalSets;
  int filledSets;
  VkDescriptorSetLayout descriptorSetLayout;
  void createDescriptorSetLayout(VkDevice device);
  void destroy(VkDevice device);
  void create(VkDevice device, uint32_t totalToStore, uint32_t duplicateCount);
  int getDescriptorSetIndex();
};
