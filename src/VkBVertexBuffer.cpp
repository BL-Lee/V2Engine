#include "VkBVertexBuffer.hpp"
#include "Vertex.hpp"
#include "VkBBuffer.hpp"
#include <stdexcept>
#include <cstring>
#include "VkBGlobals.hpp"
void
VkBVertexBuffer::destroy()
{
  vkDestroyBuffer(device, buffer, nullptr);
  vkFreeMemory(device, bufferMemory, nullptr);
  vkDestroyBuffer(device, indexBuffer, nullptr);
  vkFreeMemory(device, indexBufferMemory, nullptr);

}

void
VkBVertexBuffer::fill(
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
VkBVertexBuffer::create(VkDeviceSize initialVertexSize,
			VkDeviceSize initialIndexSize
			)
{
  VkMemoryPropertyFlags stagingMemFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VkBufferUsageFlags stagingUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  createBuffer(initialVertexSize + initialIndexSize,
	       stagingUsageFlags,
	       stagingMemFlags,
	       stagingBuffer,
	       stagingBufferMemory);
  

  VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  //For ray pass niclude storage buffer
  VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  createBuffer(initialVertexSize,
	       usageFlags,
	       memFlags,
	       buffer,
	       bufferMemory);

  VkMemoryPropertyFlags indexMemFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  VkBufferUsageFlags indexUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  createBuffer(initialIndexSize,
	       indexUsageFlags,
	       indexMemFlags,
	       indexBuffer,
	       indexBufferMemory);

  
}

void
VkBVertexBuffer::transferToDevice(VkCommandPool transientPool, VkQueue graphicsQueue)
{
  copyBuffer(stagingBuffer, buffer, sizeof(Vertex) * vertexCount,
	     transientPool, graphicsQueue,
	     0,//src offset
	     0//dst offset
	     );
  copyBuffer(stagingBuffer, indexBuffer, sizeof(uint32_t) * indexCount,
	     transientPool, graphicsQueue,
	     sizeof(Vertex) * vertexCount,//src offset
	     0//dst offset
	     );

  //Delete right after, for now but idk if ill need it again in a real example
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}
