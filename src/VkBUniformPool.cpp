#include "VkBGlobals.hpp"

#include "VkBUniformPool.hpp"
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

void VkBUniformPool::create(uint32_t totalToStore, uint32_t duplicateCount) {

  filledSets = 0;
  imagesInFlight = duplicateCount;

  totalSets = duplicateCount * totalToStore;
  
  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = (uint32_t)totalSets;

  //TODO: Should intuit this from the descriptorSetLayouts
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(totalSets);
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(totalSets);
  
  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = (uint32_t)poolSizes.size(); //Number of pools to make
  poolInfo.pPoolSizes = poolSizes.data(); //Size info
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

void VkBUniformPool::createDescriptorSetLayout() {

  //Might need to pass these in.. to say what the layout of these uniforms will be
  
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0; //It'll say where in the shader
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr; // For texture stuff

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
    uboLayoutBinding,
    samplerLayoutBinding
  };
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = (uint32_t)bindings.size();
  layoutInfo.pBindings = bindings.data();
  
  if (vkCreateDescriptorSetLayout(device,
				  &layoutInfo,
				  nullptr,
				  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void VkBUniformPool::destroy()
{
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}
