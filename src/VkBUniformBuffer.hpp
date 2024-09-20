#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include "VkBUniformPool.hpp"
class VkBUniformBuffer
{
  
public:

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;

  int framesInFlight; //Assume one per frame in flight
  size_t uniformSize;

  int indexIntoPool;

  
  void destroy();

  void createUniformBuffers(VkPhysicalDevice physicalDevice,
					    VkDevice device,
					    size_t uniformSize,
					      int maxFramesInFlight);
  
  void allocateDescriptorSets(VkDevice device, VkBUniformPool& descriptorPool,
			      VkImageView textureImageView, //temp
			      VkSampler textureSampler //temp
);

};
