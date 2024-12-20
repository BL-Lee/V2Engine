#include "VkBVertexBuffer.hpp"
#include "Vertex.hpp"
#include "VkBBuffer.hpp"
#include <stdexcept>
#include <cstring>
#include "VkBGlobals.hpp"
void
VkBVertexBuffer::destroy()
{
  vkDestroyBuffer(device, vertexBuffer, nullptr);
  vkFreeMemory(device, vertexBufferMemory, nullptr);
  if (indexed)
    {
      vkDestroyBuffer(device, indexBuffer, nullptr);
      vkFreeMemory(device, indexBufferMemory, nullptr);
    }
  //Could delete earlier if we know we wont send more to this buffer
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkUnmapMemory(device, stagingBufferMemory);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

}

void
VkBVertexBuffer::fill(const Vertex* vertices, uint32_t vCount, uint32_t* vertexOffset)
{
  if (indexed)
    {
      fprintf(stderr, "WARNING: attempting to fill indexed  vertex buffer with no indices\n");
    }
  
  memcpy(stagingBufferMapped, vertices, vCount * sizeof(Vertex));
  meshesMapped++;

  //Transfer
  copyBuffer(stagingBuffer, vertexBuffer, sizeof(Vertex) * vCount,
	     transientCommandPool, graphicsQueue,
	     0,//src offset
	     vertexCount * sizeof(Vertex)//dst offset
	     );
  
  vertexCount += vCount;

  *vertexOffset = vertexCount - vCount;
}


void
VkBVertexBuffer::fill(const Vertex* vertices, uint32_t vCount,
		      const uint32_t* indices, uint32_t iCount,
		      uint32_t* indexOffset, uint32_t* vertexOffset
		      )
{
  if (!indexed)
    {
      fprintf(stderr, "WARNING: attempting to fill unindexed vertex buffer with indices\n");
    }
  
  memcpy(stagingBufferMapped, vertices, vCount * sizeof(Vertex));
  void* offset = (void*)((uint8_t*)stagingBufferMapped + (vCount * sizeof(Vertex)));
  memcpy(offset, indices, iCount * sizeof(uint32_t));

  meshesMapped++;

  //Transfer
  copyBuffer(stagingBuffer, vertexBuffer, sizeof(Vertex) * vCount,
	     transientCommandPool, graphicsQueue,
	     0,//src offset
	     vertexCount * sizeof(Vertex)//dst offset
	     );
  copyBuffer(stagingBuffer, indexBuffer, sizeof(uint32_t) * iCount,
	     transientCommandPool, graphicsQueue,
	     sizeof(Vertex) * vCount,//src offset
	     indexCount * sizeof(uint32_t)//dst offset
	     );
  
  vertexCount += vCount;
  indexCount += iCount;

  *indexOffset = indexCount - iCount; //Index into buffer for drawing
  *vertexOffset = vertexCount - vCount;
}

void
VkBVertexBuffer::fill(const LineVertex* vertices, uint32_t vCount, uint32_t* vertexOffset)
{
  if (indexed)
    {
      fprintf(stderr, "WARNING: attempting to fill indexed Line vertex buffer with no indices\n");
    }
  
  memcpy(stagingBufferMapped, vertices, vCount * sizeof(LineVertex));
  meshesMapped++;

  //Transfer
  copyBuffer(stagingBuffer, vertexBuffer, sizeof(LineVertex) * vCount,
	     transientCommandPool, graphicsQueue,
	     0,//src offset
	     vertexCount * sizeof(LineVertex)//dst offset
	     );
  
  vertexCount += vCount;

  *vertexOffset = vertexCount - vCount;
}

void
VkBVertexBuffer::fill(const LineVertex* vertices, uint32_t vCount,
		      const uint32_t* indices, uint32_t iCount,
		      uint32_t* indexOffset, uint32_t* vertexOffset
		      )
{
  if (!indexed)
    {
      fprintf(stderr, "WARNING: attempting to fill unindexed Line vertex buffer with indices\n");
    }
  memcpy(stagingBufferMapped, vertices, vCount * sizeof(LineVertex));
  void* offset = (void*)((uint8_t*)stagingBufferMapped + (vCount * sizeof(LineVertex)));
  memcpy(offset, indices, iCount * sizeof(uint32_t));

  meshesMapped++;

  //Transfer
  copyBuffer(stagingBuffer, vertexBuffer, sizeof(LineVertex) * vCount,
	     transientCommandPool, graphicsQueue,
	     0,//src offset
	     vertexCount * sizeof(LineVertex)//dst offset
	     );
  copyBuffer(stagingBuffer, indexBuffer, sizeof(uint32_t) * iCount,
	     transientCommandPool, graphicsQueue,
	     sizeof(LineVertex) * vCount,//src offset
	     indexCount * sizeof(uint32_t)//dst offset
	     );
  
  vertexCount += vCount;
  indexCount += iCount;

  *indexOffset = indexCount - iCount; //Index into buffer for drawing
  *vertexOffset = vertexCount - vCount;
}
void
VkBVertexBuffer::create(VkDeviceSize initialVertexSize)
{
  indexed = false;
  initialize(initialVertexSize, 0);
}
void
VkBVertexBuffer::create(VkDeviceSize initialVertexSize,
			VkDeviceSize initialIndexSize)
{
  indexed = true;
  initialize(initialVertexSize, initialIndexSize);
}



void
VkBVertexBuffer::initialize(VkDeviceSize initialVertexSize,
			VkDeviceSize initialIndexSize)
{
  meshesMapped = 0;
  indexCount = 0;
  vertexCount = 0;
  maxVertexCount = initialVertexSize / sizeof(Vertex);
  maxIndexCount = initialIndexSize / sizeof(uint32_t);
  stagingBufferSize = initialIndexSize + initialVertexSize;
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
	       vertexBuffer,
	       vertexBufferMemory);
  if (indexed)
    {
      VkMemoryPropertyFlags indexMemFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      VkBufferUsageFlags indexUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
      createBuffer(initialIndexSize,
		   indexUsageFlags,
		   indexMemFlags,
		   indexBuffer,
		   indexBufferMemory);
    }
  vkMapMemory(device, stagingBufferMemory,
		0,
	      stagingBufferSize,
		0, &stagingBufferMapped);

}

