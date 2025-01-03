#include "Material.hpp"
#include "VkBBuffer.hpp"
#include "VkBSingleCommandBuffer.hpp"
void MaterialHandler::init(size_t initialMaterialCount)
{
  maxMaterialSize = initialMaterialCount;
  occupiedMaterials = (bool*)calloc(initialMaterialCount, sizeof(bool));

  VkMemoryPropertyFlags stagingMemFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VkBufferUsageFlags stagingUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  stagingBufferSize = sizeof(Material);
  createBuffer(stagingBufferSize,
	       stagingUsageFlags,
	       stagingMemFlags,
	       stagingBuffer,
	       stagingBufferMemory);
  
  vkMapMemory(device, stagingBufferMemory,
		0,
	      stagingBufferSize,
		0, &stagingBufferMapped);


  VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  //For ray pass niclude storage buffer
  VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  createBuffer(initialMaterialCount * sizeof(Material),
	       usageFlags,
	       memFlags,
	       materialBuffer,
	       materialBufferMemory);


  uniformPool.create(1, framesInFlight, 0, true);
  uniformPool.addStorageBuffer(0, initialMaterialCount * sizeof(Material));
  uniformPool.createDescriptorSetLayout();

  
  uniform.allocateDescriptorSets(&uniformPool, nullptr, &materialBuffer);
  
}
void MaterialHandler::destroy()
{
  vkDestroyBuffer(device, materialBuffer, nullptr);
  vkFreeMemory(device, materialBufferMemory, nullptr);
  //Could delete earlier if we know we wont send more to this buffer
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkUnmapMemory(device, stagingBufferMemory);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

  uniform.destroy();
  uniformPool.destroy();
  free(occupiedMaterials);
}
uint32_t MaterialHandler::fill(Material* m)
{
  uint32_t bufferIndex = 0;
  for (uint32_t i = 0; i < maxMaterialSize; i++)
    {
      if (!occupiedMaterials[i])
	{
	  occupiedMaterials[i] = true;
	  bufferIndex = i;
	  break;
	}
    }
  
  memcpy(stagingBufferMapped, m, sizeof(Material));
  
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = bufferIndex * sizeof(Material);
  copyRegion.size = sizeof(Material);
  vkCmdCopyBuffer(commandBuffer, stagingBuffer, materialBuffer, 1, &copyRegion);
    
  //Send the transfer
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
    
  vKEndSingleTimeCommandBuffer(commandBuffer);
  return bufferIndex;
}
