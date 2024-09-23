#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vector>
class VkBUniformPool {

public:

  /*
Create
-> create buffers, map the memory
Add Image/Add Buffer
initialize()
-> (sets the descriptorsetlayouts from the addimage/buffer)
-> vkcreatediscriptorpool (make from descriptorpoolsize)
-> allocateDescriptorSet


   */
  
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;
  size_t uniformSize;
  
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  int imagesInFlight;
  int totalSets;
  int filledSets;
  VkDescriptorSetLayout descriptorSetLayout;
  std::vector<VkDescriptorSetLayoutBinding> descriptorLayoutBindings;
  std::vector<size_t> bufferOffsets;
  std::vector<size_t> bufferSizes;
  
  int currentBufferOffset = 0;
  void initialize();
  void createDescriptorSetLayout();
  VkDescriptorBufferInfo getBufferInfo(int descriptorIndex,
				       int indexIntoDescriptor, int);
  VkDescriptorImageInfo getImageBufferInfo(VkSampler sampler,
					    VkImageView imageView);
  void addBuffer(int, size_t);
  void addImage(int dstBinding);
  void destroy();
  void create(uint32_t totalToStore, uint32_t duplicateCount, size_t sizeOfUniform);
  int getDescriptorSetIndex();
};
