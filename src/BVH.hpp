#pragma once

#include <glm/glm.hpp>
#include "VkBGlobals.hpp"
#include "Vertex.hpp"
#include "OBJLoader.hpp"
struct BVHNode
{
  alignas(sizeof(glm::vec4)) glm::vec3 min;
  alignas(sizeof(glm::vec4)) glm::vec3 max;
  int modelMatrixIndex; // if not -1 then its a root node
  uint32_t triangleCount; //if 0 then other int is child index, nonzero then leaf and its traingle index
  uint32_t triangleOrChildIndex;
  
  void init();
  void grow(glm::vec3 pos);
};

class BVH
{
public:
  uint32_t nodeCount;
  uint32_t nodeCapacity;

  VkDeviceMemory stagingBufferMemory;
  BVHNode* stagingNodeData; //mapped staging buffer
  VkBuffer stagingBuffer;
  size_t stagingBufferSize;

  VkBuffer deviceBuffer;
  VkDeviceMemory deviceBufferMemory;

  void init(int maxNodeCount);
  void addModel(Model* model);
  void transferBVHData();
  void destroy();
};
