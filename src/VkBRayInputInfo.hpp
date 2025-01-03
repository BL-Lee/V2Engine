#pragma once

#include "VkBVertexBuffer.hpp"
#include "VkBUniformPool.hpp"
#include "VkBUniformBuffer.hpp"
#include "BVH.hpp"
#include "OBJLoader.hpp"
class VkBRayInputInfo
{
public:
  VkBVertexBuffer vertexBuffer;
  VkBuffer AABBBuffer;

  VkBUniformPool assemblerPool;
  VkBUniformBuffer assemblerBuffer;

  BVH bvh;

  int matrixCount;
  int maxMatrixCount;
  VkDeviceMemory stagingMatrixBufferMemory;
  glm::mat4* stagingMatrixData;
  VkBuffer stagingMatrixBuffer;
  size_t stagingMatrixBufferSize;

  VkBuffer deviceMatrixBuffer;
  VkDeviceMemory deviceMatrixBufferMemory;
  
  void init();
  void destroy();
  void addModel(Model* model);
  void updateModelMatrix(Model* model);
  void transferMatrixData();
  
};
