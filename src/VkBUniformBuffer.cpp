#include "VkBUniformBuffer.hpp"
#include "VkBBuffer.hpp"


#include <stdexcept>
void VkBUniformBuffer::destroy(VkDevice device) {
  for (size_t i = 0; i < uniformBuffers.size(); i++) { //TODO: should just make this a constant
    vkDestroyBuffer(device, uniformBuffers[i], nullptr);
    vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
  }
}


void VkBUniformBuffer::createUniformBuffers(VkPhysicalDevice physicalDevice,
					    VkDevice device,
					    size_t uSize,
					    int maxFramesInFlight) {
  VkDeviceSize bufferSize = uSize;
  uniformSize = uSize;
  framesInFlight = maxFramesInFlight;
  
  uniformBuffers.resize(maxFramesInFlight);
  uniformBuffersMemory.resize(maxFramesInFlight);
  uniformBuffersMapped.resize(maxFramesInFlight);
  
  for (size_t i = 0; i < maxFramesInFlight; i++) {
    createBuffer(physicalDevice, device,
		 bufferSize,
		 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		 uniformBuffers[i],
		 uniformBuffersMemory[i]);
	
    vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
  }
}

//TODO: to abstract
void VkBUniformBuffer::allocateDescriptorSets(VkDevice device, VkBUniformPool& descriptorPool)
  {
    indexIntoPool = descriptorPool.getDescriptorSetIndex();
    for (size_t i = 0; i < framesInFlight; i++) {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = uniformSize;

      VkWriteDescriptorSet descriptorWrite{};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = descriptorPool.descriptorSets[indexIntoPool + i];
      descriptorWrite.dstBinding = 0;//Where in the shader
      descriptorWrite.dstArrayElement = 0; //Descriptors can be arrays, so say the first
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;
      descriptorWrite.pImageInfo = nullptr; // Optional
      descriptorWrite.pTexelBufferView = nullptr; // Optional
      
      vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
  }
