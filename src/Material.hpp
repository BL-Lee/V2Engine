#pragma once

#include "VkBGlobals.hpp"
#include "VkBUniformPool.hpp"
#include "VkBUniformBuffer.hpp"
#include <iostream>
#include "glm/glm.hpp"
class Material {
public:
  glm::vec4 colour;
  glm::vec2 atlasMin;
  glm::vec2 atlasMax;
};


class MaterialHandler
{
public:
  
  VkBuffer materialBuffer;
  VkBuffer stagingBuffer;
  void* stagingBufferMapped;

  VkDeviceMemory materialBufferMemory;
  VkDeviceMemory stagingBufferMemory;
  
  size_t stagingBufferSize;
  
  uint32_t maxMaterialSize;
  bool* occupiedMaterials;

  VkBUniformPool uniformPool;
  VkBUniformBuffer uniform;

  void init(size_t initialMaterialCount);
  void destroy();
  uint32_t fill(Material*);
 
};
