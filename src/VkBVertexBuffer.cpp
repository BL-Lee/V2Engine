#include "VkBVertexBuffer.hpp"
#include "Vertex.hpp"
#include "VkBSingleCommandBuffer.hpp"
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
  if (vCount + vertexCount > maxVertexCount)
    {
      reallocBuffers(true, false);
    }
    
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
  while (vCount + vertexCount > maxVertexCount || iCount + indexCount > maxIndexCount)
    {
      bool doVertices = vCount + vertexCount >= maxVertexCount;
      bool doIndices = iCount + indexCount >= maxIndexCount;
      reallocBuffers(doVertices, doIndices);
    }
  if (!indexed)
    {
      fprintf(stderr, "WARNING: attempting to fill unindexed vertex buffer with indices\n");
    }

  if (vCount * sizeof(Vertex) + iCount * sizeof(uint32_t) < stagingBufferSize)
    {
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

    }
  else //Staging buffer too small for the whole thing, move in blocks
    {
      size_t vOffset = 0;
      size_t maxVertsPerBlock = stagingBufferSize / sizeof(Vertex);
      while (vOffset < vCount)
	{
	  size_t vertCountToMove = vOffset + maxVertsPerBlock >= vCount ? vCount - vOffset : maxVertsPerBlock;

	  memcpy(stagingBufferMapped, vertices + vOffset, vertCountToMove * sizeof(Vertex));
	  copyBuffer(stagingBuffer, vertexBuffer, vertCountToMove * sizeof(Vertex),
		     transientCommandPool, graphicsQueue,
		     0,//src offset
		     vertexCount * sizeof(Vertex)//dst offset
		     );
	  vertexCount += vertCountToMove;
	  vOffset += vertCountToMove;
	}

      size_t iOffset = 0;
      size_t maxIndicesPerBlock = stagingBufferSize / sizeof(uint32_t);
      while (iOffset < iCount)
	{
	  size_t indexCountToMove = iOffset + maxIndicesPerBlock >= iCount ? iCount - iOffset : maxIndicesPerBlock;

	  memcpy(stagingBufferMapped, indices + iOffset, indexCountToMove * sizeof(uint32_t));
	  copyBuffer(stagingBuffer, indexBuffer, indexCountToMove * sizeof(uint32_t),
		     transientCommandPool, graphicsQueue,
		     0,//src offset
		     indexCount * sizeof(uint32_t)//dst offset
		     );
	  indexCount += indexCountToMove;
	  iOffset += indexCountToMove;
	}

    }

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

//This will probably cause issues if we're in the middle of a render pass
//Since all the command buffers need to be rebound
//Not to mention the single time command buffer blocks
void VkBVertexBuffer::reallocBuffers(bool doVertices, bool doIndices)
{
  printf("Reallocing Buffers: \nVertices: %zd -> %zd Indices: %d -> %d \n",
	 maxVertexCount, doVertices ? maxVertexCount * 2 : maxVertexCount,
	 maxIndexCount, doIndices ? maxIndexCount * 2 : maxIndexCount); 
  if (!doVertices && !doIndices)
    return;
  VkDeviceMemory tempVertexBufferMemory;
  VkBuffer tempVertexBuffer;
  VkDeviceMemory tempIndexBufferMemory;
  VkBuffer tempIndexBuffer;
  
  VkCommandBuffer copyCommands = vKBeginSingleTimeCommandBuffer();
  if (doVertices)
    {
      VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      //For ray pass niclude storage buffer
      VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
	VK_BUFFER_USAGE_TRANSFER_DST_BIT |
	VK_BUFFER_USAGE_TRANSFER_SRC_BIT |    
	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;


      maxVertexCount = maxVertexCount * 2;
      createBuffer(maxVertexCount * sizeof(Vertex),
		   usageFlags,
		   memFlags,
		   tempVertexBuffer,
		   tempVertexBufferMemory);
      if (vertexCount > 0)
	{
	  VkBufferCopy bufferCopy = {};
	  bufferCopy.srcOffset = 0;
	  bufferCopy.dstOffset = 0;
	  bufferCopy.size = vertexCount * sizeof(Vertex);

	  vkCmdCopyBuffer(copyCommands,
			  vertexBuffer, tempVertexBuffer,
			  1,
			  &bufferCopy);
	}
    }

  if (doIndices)
    {
      VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      //For ray pass niclude storage buffer
      VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
	VK_BUFFER_USAGE_TRANSFER_DST_BIT |
	VK_BUFFER_USAGE_TRANSFER_SRC_BIT |    
	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

      maxIndexCount = maxIndexCount * 2;
      createBuffer(maxIndexCount * sizeof(uint32_t),
		   usageFlags,
		   memFlags,
		   tempIndexBuffer,
		   tempIndexBufferMemory);
      if (indexCount > 0)
	{
	  VkBufferCopy bufferCopy = {};
	  bufferCopy.srcOffset = 0;
	  bufferCopy.dstOffset = 0;
	  bufferCopy.size = indexCount * sizeof(uint32_t);
  
	  vkCmdCopyBuffer(copyCommands,
			  indexBuffer, tempIndexBuffer,
			  1,
			  &bufferCopy);
	}
    }


  		  
  vKEndSingleTimeCommandBuffer(copyCommands);
  if (doVertices)
    {
      vkDestroyBuffer(device, vertexBuffer, nullptr);
      vkFreeMemory(device, vertexBufferMemory, nullptr);
      vertexBuffer = tempVertexBuffer;
      vertexBufferMemory = tempVertexBufferMemory;
    }
  if (doIndices)
    {
      vkDestroyBuffer(device, indexBuffer, nullptr);
      vkFreeMemory(device, indexBufferMemory, nullptr);
      indexBuffer = tempIndexBuffer;
      indexBufferMemory = tempIndexBufferMemory;
    }
  
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
  VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT |    
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  createBuffer(initialVertexSize,
	       usageFlags,
	       memFlags,
	       vertexBuffer,
	       vertexBufferMemory);
  if (indexed)
    {
      VkMemoryPropertyFlags indexMemFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      VkBufferUsageFlags indexUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
	VK_BUFFER_USAGE_TRANSFER_DST_BIT |
	VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
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

