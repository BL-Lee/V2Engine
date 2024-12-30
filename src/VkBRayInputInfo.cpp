#include "VkBRayInputInfo.hpp"
#include "VkBBuffer.hpp"
#define TEMP_VERTEX_MAX 10000
void VkBRayInputInfo::init()
{
  //TODO: should know how to up this size dynamically, and everything else
  
  vertexBuffer.create(sizeof(Vertex) * TEMP_VERTEX_MAX, sizeof(uint32_t) * TEMP_VERTEX_MAX);
  assemblerPool.create(1, framesInFlight, 0, true);
  assemblerPool.addStorageBuffer(0,(TEMP_VERTEX_MAX) * sizeof(Vertex));
  assemblerPool.addStorageBuffer(1, (TEMP_VERTEX_MAX) * sizeof(uint32_t));
  assemblerPool.addStorageBuffer(3, (8)*sizeof(BVHNode));
  assemblerPool.addStorageBuffer(4, (16)*sizeof(glm::mat4));

  assemblerPool.addImage(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  assemblerPool.createDescriptorSetLayout();

  bvh.init();


  matrixCount = 0;
  VkMemoryPropertyFlags stagingMemFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VkBufferUsageFlags stagingUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  stagingMatrixBufferSize = sizeof(glm::mat4) * 16;
  createBuffer(stagingMatrixBufferSize, stagingUsageFlags, stagingMemFlags, stagingMatrixBuffer, stagingMatrixBufferMemory);
  vkMapMemory(device, stagingMatrixBufferMemory,
	      0,
	      stagingMatrixBufferSize,
	      0, &(void*)stagingMatrixData);

  VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  //For ray pass niclude storage buffer
  VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  createBuffer(stagingMatrixBufferSize,
	       usageFlags,
	       memFlags,
	       deviceMatrixBuffer,
	       deviceMatrixBufferMemory);
  
}

//TODO: model matrices, 
void VkBRayInputInfo::addModel(Model* model)
{
  model->indexIntoModelMatrixBuffer = matrixCount;
  bvh.addModel(model);
  updateModelMatrix(model);
  matrixCount += 2;
}

void VkBRayInputInfo::updateModelMatrix(Model* model)
{
  stagingMatrixData[model->indexIntoModelMatrixBuffer] = model->modelMatrix;
  stagingMatrixData[model->indexIntoModelMatrixBuffer + 1] = glm::inverse(model->modelMatrix);
}

void VkBRayInputInfo::destroy()
{
  vertexBuffer.destroy();
  assemblerBuffer.destroy();
  assemblerPool.destroy();
  bvh.destroy();
  
  vkDestroyBuffer(device, deviceMatrixBuffer, nullptr);
  vkFreeMemory(device, deviceMatrixBufferMemory, nullptr);

  //Could delete earlier if we know we wont send more to this buffer
  vkDestroyBuffer(device, stagingMatrixBuffer, nullptr);
  vkUnmapMemory(device, stagingMatrixBufferMemory);
  vkFreeMemory(device, stagingMatrixBufferMemory, nullptr);

}
void VkBRayInputInfo::transferMatrixData()
{
    //Transfer
  copyBuffer(stagingMatrixBuffer, deviceMatrixBuffer, sizeof(glm::mat4) * matrixCount,
	     transientCommandPool, graphicsQueue,
	     0,//src offset
	     0);

}
