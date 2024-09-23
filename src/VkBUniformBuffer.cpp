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

//Location to memcpy to when updating
void* VkBUniformBuffer::getBufferMemoryLocation(int imageIndex, int bufferIndex)
{
  return (void*)((uint8_t*)uniformPool.uniformBuffersMapped[imageIndex] + bufferInfo[bufferIndex].offset);
}
//TODO: to abstract
void VkBUniformBuffer::allocateDescriptorSets(VkBUniformPool& descriptorPool,
					      VkImageView* textureImageView,  //arrays
					      VkSampler* textureSampler 
					      )
  {
    uniformPool = descriptorPool;
    indexIntoPool = descriptorPool.getDescriptorSetIndex();// This gets offset into pool

    for (size_t i = 0; i < descriptorPool.imagesInFlight; i++) {
      uint32_t imgCount = 0;      
      std::vector<VkWriteDescriptorSet> descriptorWrites{};
      descriptorWrites.resize(descriptorPool.descriptorLayoutBindings.size());
      
      for (int j = 0; j < descriptorPool.descriptorLayoutBindings.size(); j++)
	{
	  VkDescriptorSetLayoutBinding binding = descriptorPool.descriptorLayoutBindings[j];

	  if (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	    {
	      VkDescriptorImageInfo t_imageInfo = descriptorPool.getImageBufferInfo(textureSampler[imgCount], textureImageView[imgCount]);
	      
	      if (i == 0)
		  imageInfo.push_back(t_imageInfo);

	      descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	      descriptorWrites[j].dstSet = descriptorPool.descriptorSets[i][indexIntoPool];
	      descriptorWrites[j].dstBinding = binding.binding;//Where in the shader
	      descriptorWrites[j].dstArrayElement = 0; //Descriptors can be arrays, so say the first
	      descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	      descriptorWrites[j].descriptorCount = 1;
	      descriptorWrites[j].pImageInfo = &t_imageInfo; // Optional
	      imgCount++;
	    }
	  else
	    {
	      
	      VkDescriptorBufferInfo t_bufferInfo = descriptorPool.getBufferInfo(indexIntoPool, 0, i); //Since just one buffer so far, but would switch 2nd one to which buffer this is in the uniform
	      if (i == 0)
		  bufferInfo.push_back(t_bufferInfo);

	      descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	      descriptorWrites[j].dstSet = descriptorPool.descriptorSets[i][indexIntoPool];
	      descriptorWrites[j].dstBinding = binding.binding;//Where in the shader
	      descriptorWrites[j].dstArrayElement = 0; //Descriptors can be arrays, so say the first
	      descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	      descriptorWrites[j].descriptorCount = 1;
	      descriptorWrites[j].pBufferInfo = &t_bufferInfo;
	      descriptorWrites[j].pImageInfo = nullptr; // Optional
	      descriptorWrites[j].pTexelBufferView = nullptr; // Optional
	    }
	}
      vkUpdateDescriptorSets(device,
			     (uint32_t)descriptorWrites.size(),
			     descriptorWrites.data(),
			     0, nullptr);
    }
  }
