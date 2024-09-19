#include "VkBBuffer.hpp"
#include <stdexcept>
#include "VkBGlobals.hpp"
#include "VkBSingleCommandBuffer.hpp"
uint32_t findMemoryType(uint32_t typeFilter,
			VkMemoryPropertyFlags properties)
{
  //Contains info like, which heaps and tpyes (which device and where the memory is)
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    //typefilter is bit field of memory types we're okay with for this
    if ((typeFilter & (1 << i)) &&
	(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
  
}

void createBuffer(VkDeviceSize size,
		  VkBufferUsageFlags usage,
		  VkMemoryPropertyFlags properties,
		  VkBuffer& buffer,
		  VkDeviceMemory& bufferMemory) {
  
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
					     properties);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

//Command pool should be transientCommandPool
void copyBuffer(VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size,
		VkCommandPool commandPool,
		VkQueue graphicsQueue,
		uint32_t srcOffset,
		uint32_t dstOffset
		) {

  //NOTE: this should be handled a bit differently.
  //"custom allocator that splits up a single allocation among many different objects by using the offset parameters that we've seen in many functions."
  //See bottom of https://vulkan-tutorial.com/en/Vertex_buffers/Staging_buffer
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = srcOffset;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
  //Send the transfer
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
    
  vKEndSingleTimeCommandBuffer(commandBuffer);  
}
