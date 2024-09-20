#include "VkBUniformBuffer.hpp"
#include "VkBBuffer.hpp"
#include "VkBGlobals.hpp"
#include <array>
#include <stdexcept>
void VkBUniformBuffer::destroy() {
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
    createBuffer(bufferSize,
		 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		 uniformBuffers[i],
		 uniformBuffersMemory[i]);
	
    vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
  }
}

//TODO: to abstract
void VkBUniformBuffer::allocateDescriptorSets(VkDevice device,
					      VkBUniformPool& descriptorPool,
					      VkImageView textureImageView, //temp
					      VkSampler textureSampler //temp
					      )
  {
    indexIntoPool = descriptorPool.getDescriptorSetIndex();
    for (size_t i = 0; i < framesInFlight; i++) {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = uniformSize;

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = textureImageView;
      imageInfo.sampler = textureSampler;

      std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
      
      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = descriptorPool.descriptorSets[indexIntoPool + i];
      descriptorWrites[0].dstBinding = 0;//Where in the shader
      descriptorWrites[0].dstArrayElement = 0; //Descriptors can be arrays, so say the first
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;
      descriptorWrites[0].pImageInfo = nullptr; // Optional
      descriptorWrites[0].pTexelBufferView = nullptr; // Optional

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = descriptorPool.descriptorSets[indexIntoPool + i];
      descriptorWrites[1].dstBinding = 1;//Where in the shader
      descriptorWrites[1].dstArrayElement = 0; //Descriptors can be arrays, so say the first
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pImageInfo = &imageInfo; // Optional
      
      vkUpdateDescriptorSets(device,
			     (uint32_t)descriptorWrites.size(),
			     descriptorWrites.data(),
			     0, nullptr);
    }
  }
