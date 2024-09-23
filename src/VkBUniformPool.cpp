#include "VkBGlobals.hpp"

#include "VkBUniformPool.hpp"
#include "VkBBuffer.hpp"
#include <stdexcept>
#include <array>
//General consensus is to break up descriptor sets (buffers?) into different sections
/*

  generally:
    global
      per render (projection, view matrices etc)
        per material 
          per mesh

*/

//Assume one per image in flight

void VkBUniformPool::create(uint32_t totalToStore, uint32_t totalFrames,
			    VkDeviceSize sizeOfUniform
			    ) {

  //Should global this
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(physicalDevice, &props);

  uniformSize = sizeOfUniform < props.limits.minUniformBufferOffsetAlignment ? props.limits.minUniformBufferOffsetAlignment : sizeOfUniform;

  filledSets = 0;
  imagesInFlight = totalFrames;
  totalSets = totalToStore;
  imageCount = 0;
  //Allocate buffer and map to pointer
  uniformBuffers.resize(totalFrames);
  uniformBuffersMemory.resize(totalFrames);
  uniformBuffersMapped.resize(totalFrames);

  descriptorSets.resize(totalFrames);
  //descriptorLayoutBindings.resize(totalFrames);
  
  for (size_t i = 0; i < totalFrames; i++) {
    createBuffer(uniformSize * totalToStore,
		 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		 uniformBuffers[i],
		 uniformBuffersMemory[i]);
	
    vkMapMemory(device, uniformBuffersMemory[i], 0, uniformSize * totalToStore, 0, &uniformBuffersMapped[i]);
  }
}

void VkBUniformPool::createDescriptorSetLayout() {
  
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = (uint32_t)descriptorLayoutBindings.size();
  layoutInfo.pBindings = descriptorLayoutBindings.data();
  
  if (vkCreateDescriptorSetLayout(device,
				  &layoutInfo,
				  nullptr,
				  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
  //clear vector
  
  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = (uint32_t)totalSets * imagesInFlight;

  std::vector<VkDescriptorPoolSize> poolSizes{};
  poolSizes.resize(descriptorLayoutBindings.size());
  for (int i = 0; i < descriptorLayoutBindings.size(); i++)
    {
      poolSizes[i].type = descriptorLayoutBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? 
	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      poolSizes[i].descriptorCount = totalSets * imagesInFlight;
    }
  
  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = (uint32_t)poolSizes.size(); //Number of pools to make
  poolInfo.pPoolSizes = poolSizes.data(); //Size info
  poolInfo.maxSets = (uint32_t)totalSets * imagesInFlight; //Total we'll go up to?

  if (vkCreateDescriptorPool(device,
			     &poolInfo,
			     nullptr,
			     &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }

  for (int i = 0; i < imagesInFlight; i++)
    {
      std::vector<VkDescriptorSetLayout> layouts(totalSets, descriptorSetLayout);
  
      VkDescriptorSetAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = descriptorPool;
      allocInfo.descriptorSetCount = (uint32_t)totalSets;
      allocInfo.pSetLayouts = layouts.data();
  
      descriptorSets[i].resize(totalSets);
      if (vkAllocateDescriptorSets(device,
				   &allocInfo,
				   descriptorSets[i].data()) != VK_SUCCESS) {
	throw std::runtime_error("failed to allocate descriptor sets!");
      }
    }
}

int VkBUniformPool::getDescriptorSetIndex()
{
  filledSets++;
  return (filledSets - 1);
}

VkDescriptorBufferInfo VkBUniformPool::getBufferInfo(int descriptorIndex, //Which buffer
						     int indexIntoDescriptor, //Which buffer in this UBO
						     int frameIdx
						     )
{
  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = uniformBuffers[frameIdx]; //TODO: resizing and chunking
  bufferInfo.offset = uniformSize * descriptorIndex + bufferOffsets[indexIntoDescriptor];
  bufferInfo.range = bufferSizes[indexIntoDescriptor];

  return bufferInfo;
}

VkDescriptorImageInfo VkBUniformPool::getImageBufferInfo(VkSampler sampler,
							  VkImageView imageView)
{
  VkDescriptorImageInfo bufferInfo{};
  bufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  bufferInfo.imageView = imageView;
  bufferInfo.sampler = sampler;
  return bufferInfo;
}



void VkBUniformPool::addBuffer(int dstBinding, size_t bufferSize)
{
  //Should global this
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(physicalDevice, &props);

  
  //Need to know -> VkBuffer location, offset and range(Size) (for buffer)
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = dstBinding; //It'll say where in the shader
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1; //To bet set when creating
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr; // For texture stuff

  size_t trueSize = bufferSize < props.limits.minUniformBufferOffsetAlignment ? props.limits.minUniformBufferOffsetAlignment : bufferSize;
  
  descriptorLayoutBindings.push_back(uboLayoutBinding);
  bufferOffsets.push_back(currentBufferOffset);
  bufferSizes.push_back(trueSize);
  currentBufferOffset += trueSize;
}
void VkBUniformPool::addImage(int dstBinding)
{
  //Need to know -> Image layout, imageview and sampler (when creating for buffer)
  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = dstBinding;
  samplerLayoutBinding.descriptorCount = 1;//To bet set when creating
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  
  descriptorLayoutBindings.push_back(samplerLayoutBinding);

  imageCount ++;
  
}

void VkBUniformPool::destroy()
{
  for (size_t i = 0; i < uniformBuffers.size(); i++) { //TODO: should just make this a constant
    vkDestroyBuffer(device, uniformBuffers[i], nullptr);
    vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
  }

  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}
