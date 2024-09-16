#include "VkBVertexBuffer.hpp"
#include "Vertex.hpp"
#include "VkBBuffer.hpp"
#include <stdexcept>
#include <cstring>
void
VkBVertexBuffer::destroy(VkDevice device)
{
  vkDestroyBuffer(device, buffer, nullptr);
  vkFreeMemory(device, bufferMemory, nullptr);
  vkDestroyBuffer(device, indexBuffer, nullptr);
  vkFreeMemory(device, indexBufferMemory, nullptr);

}

void
VkBVertexBuffer::fill(VkDevice device,
			   const Vertex* vertices, uint32_t vCount,
			   const uint32_t* indices, uint32_t iCount)
{
  
  void* tempData;
  vertexCount = vCount;
  indexCount = iCount;
  vkMapMemory(device, stagingBufferMemory,
		0,
		vCount * sizeof(Vertex) + indexCount * sizeof(uint32_t),
		0, &tempData);
  
  memcpy(tempData, vertices, vCount * sizeof(Vertex));
  void* offset = (void*)((uint8_t*)tempData + (vertexCount * sizeof(Vertex)));
  memcpy(offset, indices, indexCount * sizeof(uint32_t));
  vkUnmapMemory(device, stagingBufferMemory);
}

    



void
VkBVertexBuffer::create(VkDevice device, VkPhysicalDevice physicalDevice,
			VkDeviceSize initialVertexSize,
			VkDeviceSize initialIndexSize
			)
{
  VkMemoryPropertyFlags stagingMemFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VkBufferUsageFlags stagingUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  createBuffer(physicalDevice,
	       device,
	       initialVertexSize + initialIndexSize,
	       stagingUsageFlags,
	       stagingMemFlags,
	       stagingBuffer,
	       stagingBufferMemory);
  

  VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  createBuffer(physicalDevice,
	       device,
	       initialVertexSize,
	       usageFlags,
	       memFlags,
	       buffer,
	       bufferMemory);

  VkMemoryPropertyFlags indexMemFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  VkBufferUsageFlags indexUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  createBuffer(physicalDevice,
	       device,
	       initialIndexSize,
	       indexUsageFlags,
	       indexMemFlags,
	       indexBuffer,
	       indexBufferMemory);

  
}

void
VkBVertexBuffer::transferToDevice(VkDevice device, VkCommandPool transientPool, VkQueue graphicsQueue)
{
  copyBuffer(device, stagingBuffer, buffer, sizeof(Vertex) * vertexCount,
	     transientPool, graphicsQueue,
	     0,//src offset
	     0//dst offset
	     );
  copyBuffer(device, stagingBuffer, indexBuffer, sizeof(uint32_t) * indexCount,
	     transientPool, graphicsQueue,
	     sizeof(Vertex) * vertexCount,//src offset
	     0//dst offset
	     );

  //Delete right after, for now but idk if ill need it again in a real example
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}
