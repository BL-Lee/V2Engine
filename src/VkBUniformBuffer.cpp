#include "VkBUniformBuffer.hpp"
#include "VkBBuffer.hpp"
#include "VkBGlobals.hpp"
#include <array>
#include <stdexcept>
void VkBUniformBuffer::destroy() {
}


void VkBUniformBuffer::createUniformBuffers(size_t uSize,
					    int maxFramesInFlight) {
}

//TODO: to abstract
void VkBUniformBuffer::allocateDescriptorSets(VkBUniformPool& descriptorPool,
					      VkImageView textureImageView, //temp
					      VkSampler textureSampler //temp
					      )
  {
    indexIntoPool = descriptorPool.getDescriptorSetIndex();
    for (size_t i = 0; i < descriptorPool.imagesInFlight; i++) {
      //for (int j = 0; j < descriptorPool.
      
      VkDescriptorBufferInfo bufferInfo = descriptorPool.getBufferInfo(0,0, i);

      VkDescriptorImageInfo imageInfo = descriptorPool.getImageBufferInfo(textureSampler, textureImageView);

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
