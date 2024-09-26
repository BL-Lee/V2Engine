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
  size_t uniformSize;
  int imagesInFlight;
  int totalSets;
  int filledSets;
  int imageCount; //Images in uniform

  //Buffer Data for where it's stored
  std::vector<VkBuffer> uniformBuffers; // 1 per frame in flight for now, but should dynamically allocate
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;

  VkDescriptorPool descriptorPool;
  
  //how each of them are laid out
  VkDescriptorSetLayout descriptorSetLayout;

  //[[frame_0], [frame_1]...]
  std::vector<std::vector<VkDescriptorSet>> descriptorSets;
  std::vector<VkDescriptorSetLayoutBinding> descriptorLayoutBindings;
  std::vector<VkBuffer> storageBuffers;
  std::vector<size_t> storageBufferSizes;
  //Doesn't need to be 2d, since each frame will be parallel
  std::vector<size_t> bufferOffsets;
  std::vector<size_t> bufferSizes;
  
  size_t currentBufferOffset = 0;
  
  void initialize();
  void createDescriptorSetLayout();
  VkDescriptorBufferInfo getBufferInfo(int descriptorIndex,
				       int indexIntoDescriptor, int);
  VkDescriptorImageInfo getImageBufferInfo(VkSampler sampler,
					    VkImageView imageView);
  VkDescriptorBufferInfo getStorageBufferInfo(VkBuffer buffer, int index);
  void addBuffer(int, size_t);
  void addStorageBuffer(int dstBinding, size_t size);
  void addImage(int dstBinding);
  void destroy();
  void create(uint32_t totalToStore, uint32_t duplicateCount, size_t sizeOfUniform);
  void create(uint32_t totalToStore, uint32_t totalFrames,
	      VkDeviceSize sizeOfUniform, bool skipBufferCreation);

  int getDescriptorSetIndex();
};
