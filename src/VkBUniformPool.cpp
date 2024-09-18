#include "VkBUniformPool.hpp"
#include <stdexcept>
//Assume one per image in flight
void VkBUniformPool::create(VkDevice device, uint32_t totalToStore, uint32_t duplicateCount) {

  filledSets = 0;
  imagesInFlight = duplicateCount;

  totalSets = duplicateCount * totalToStore;
  
  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = (uint32_t)totalSets;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1; //Number of pools to make
  poolInfo.pPoolSizes = &poolSize; //Size info
  poolInfo.maxSets = (uint32_t)totalSets; //Total we'll go up to?

  if (vkCreateDescriptorPool(device,
			     &poolInfo,
			     nullptr,
			     &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }

  std::vector<VkDescriptorSetLayout> layouts(totalSets, descriptorSetLayout);
  
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = (uint32_t)totalSets;
  allocInfo.pSetLayouts = layouts.data();
  
  descriptorSets.resize(totalSets);
  if (vkAllocateDescriptorSets(device,
			       &allocInfo,
			       descriptorSets.data()) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate descriptor sets!");
  }

}

int VkBUniformPool::getDescriptorSetIndex()
{
  filledSets++;
  return (filledSets - 1) * imagesInFlight;
}

void VkBUniformPool::createDescriptorSetLayout(VkDevice device) {
  
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0; //It'll say where in the shader
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr; // For texture stuff

  //  VkDescriptorSetLayout descriptorSetLayout;
  //VkPipelineLayout pipelineLayout;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;
  
  if (vkCreateDescriptorSetLayout(device,
				  &layoutInfo,
				  nullptr,
				  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void VkBUniformPool::destroy(VkDevice device)
{
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}
