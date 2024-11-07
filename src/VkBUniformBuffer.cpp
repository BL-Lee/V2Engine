#include "VkBUniformBuffer.hpp"
#include "VkBBuffer.hpp"
#include "VkBGlobals.hpp"
#include <vector>
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
  return (void*)((uint8_t*)uniformPool->uniformBuffersMapped[imageIndex] + bufferInfo[bufferIndex].offset);
}
//TODO: to abstract
void VkBUniformBuffer::allocateDescriptorSets(VkBUniformPool* descriptorPool,
					      VkImageView* textureImageView,  //arrays
					      VkSampler* textureSampler,
					      VkBuffer* buffers
					      )
  {
    uniformPool = descriptorPool;
    indexIntoPool = descriptorPool->getDescriptorSetIndex();// This gets offset into pool
    //bufferInfo = std::vector<VkDescriptorBufferInfo>();
    for (size_t i = 0; i < descriptorPool->imagesInFlight; i++) {
      uint32_t imageCount = 0;
      uint32_t storageCount = 0;
      uint32_t bufferCount = 0;
      //The resizing changes memory locations.
      std::vector<VkWriteDescriptorSet> descriptorWrites{};
      VkDescriptorImageInfo imageInfoBuffer[16]; //TODO make dynamic?
      VkDescriptorBufferInfo bufferInfoBuffer[16];
      VkDescriptorBufferInfo storageBufferInfoBuffer[16];
      descriptorWrites.resize(descriptorPool->descriptorLayoutBindings.size());

      for (int j = 0; j < descriptorPool->descriptorLayoutBindings.size(); j++)
	{

	  VkDescriptorSetLayoutBinding binding = descriptorPool->descriptorLayoutBindings[j];
	  if (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
	      binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
	      )
	    {
	      //NOTE: requires images to be sent in order if they are in an array
	      uint32_t startingCount = imageCount;
	      for (int img = 0; img < binding.descriptorCount; img++)
		{
		  VkDescriptorImageInfo t_imageInfo{};
		  if (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		    t_imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		  else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		    t_imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		  t_imageInfo.imageView = textureImageView[imageCount];
		  t_imageInfo.sampler = textureSampler[imageCount];
		  if (i == 0)
		    imageInfo.push_back(t_imageInfo);
		  imageInfoBuffer[imageCount] = t_imageInfo;
		  imageCount++;
		}


	      descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	      descriptorWrites[j].dstSet = descriptorPool->descriptorSets[i][indexIntoPool];
	      descriptorWrites[j].dstBinding = binding.binding;//Where in the shader
	      descriptorWrites[j].dstArrayElement = 0; //Descriptors can be arrays, so say the first
	      descriptorWrites[j].descriptorType = binding.descriptorType;
	      descriptorWrites[j].descriptorCount = binding.descriptorCount;
	      descriptorWrites[j].pImageInfo = &imageInfoBuffer[startingCount]; 

	    }
	  else if  (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
	    {
	      
	      VkDescriptorBufferInfo t_bufferInfo = descriptorPool->getBufferInfo(indexIntoPool, 0, i); //Since just one buffer so far, but would switch 2nd one to which buffer this is in the uniform
	      if (i == 0)
		  bufferInfo.push_back(t_bufferInfo);

	      bufferInfoBuffer[bufferCount] = t_bufferInfo;
	      descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	      descriptorWrites[j].dstSet = descriptorPool->descriptorSets[i][indexIntoPool];
	      descriptorWrites[j].dstBinding = binding.binding;//Where in the shader
	      descriptorWrites[j].dstArrayElement = 0; //Descriptors can be arrays, so say the first
	      descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	      descriptorWrites[j].descriptorCount = 1;
	      descriptorWrites[j].pBufferInfo = &bufferInfoBuffer[bufferCount];
	    }
	  else if  (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
	    {
	      
	      VkDescriptorBufferInfo t_bufferInfo = descriptorPool->getStorageBufferInfo(buffers[storageCount], storageCount); 
	      if (i == 0)
		  bufferInfo.push_back(t_bufferInfo);

	      storageBufferInfoBuffer[storageCount] = t_bufferInfo;
	      descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	      descriptorWrites[j].dstSet = descriptorPool->descriptorSets[i][indexIntoPool];
	      descriptorWrites[j].pBufferInfo = &storageBufferInfoBuffer[storageCount];
	      descriptorWrites[j].dstBinding = binding.binding;//Where in the shader
	      descriptorWrites[j].dstArrayElement = 0; //Descriptors can be arrays, so say the first
	      descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	      descriptorWrites[j].descriptorCount = 1;
	      storageCount++;
	    }

	}
      vkUpdateDescriptorSets(device,
			     (uint32_t)descriptorWrites.size(),
			     descriptorWrites.data(),
			     0, nullptr);
    }
  }
