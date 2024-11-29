
#include "BVH.hpp"
#include "VkBBuffer.hpp"
#include <limits>
#include <iostream>
void BVHNode::init()
{
  modelMatrixIndex = -1;
  min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
  max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

void BVHNode::grow(glm::vec3 pos)
{
  min = glm::min(pos, min);
  max = glm::max(pos, max);
}

void BVH::init()
{
  nodeCount = 0;
  VkMemoryPropertyFlags stagingMemFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VkBufferUsageFlags stagingUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  size_t stagingBufferSize = sizeof(BVHNode) * 10;
  createBuffer(stagingBufferSize, stagingUsageFlags, stagingMemFlags, stagingBuffer, stagingBufferMemory);
  vkMapMemory(device, stagingBufferMemory,
	      0,
	      stagingBufferSize,
	      0, &(void*)stagingNodeData);

  VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  //For ray pass niclude storage buffer
  VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  createBuffer(stagingBufferSize,
	       usageFlags,
	       memFlags,
	       deviceBuffer,
	       deviceBufferMemory);
}

void BVH::destroy()
{
  vkDestroyBuffer(device, deviceBuffer, nullptr);
  vkFreeMemory(device, deviceBufferMemory, nullptr);

  //Could delete earlier if we know we wont send more to this buffer
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkUnmapMemory(device, stagingBufferMemory);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
  
}

void BVH::addModel(Model* model)
{
  BVHNode* node = &stagingNodeData[nodeCount++];
  node->init();
  for (int i = 0; i < model->vertexCount; i++) //assume unindexed for now
    {
      node->grow(model->vertices[i].pos);
    }
  node->triangleCount = model->vertexCount;
  node->triangleOrChildIndex = model->vertexOffset;
  node->modelMatrixIndex = model->indexIntoModelMatrixBuffer;
}

void BVH::transferBVHData()
{
  std::cout << "TRANSFERING BVH: " << nodeCount << " nodes" << std::endl;
  for (int i = 0; i < nodeCount; i++)
    {
      std::cout << "BVHNode: " << i
		<< "/" << nodeCount <<  " count: " <<
	stagingNodeData[i].triangleCount << " startIdx: " <<
	stagingNodeData[i].triangleOrChildIndex
		<< std::endl;
    }
  //Transfer
  copyBuffer(stagingBuffer, deviceBuffer, sizeof(BVHNode) * nodeCount,
	     transientCommandPool, graphicsQueue,
	     0,//src offset
	     0);
}
  
