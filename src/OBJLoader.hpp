#pragma once
#include "VkBGlobals.hpp"
#include "VkBVertexBuffer.hpp"
#include "VkBTexture.hpp"
#include "VkBUniformBuffer.hpp"
class Model{
public:
  
  VkBTexture textures; //Just diffuse for now
  VkBUniformBuffer modelUniform;
  size_t vertexCount;
  size_t indexCount;
  uint32_t startIndex;
  uint32_t vertexOffset;
  ~Model();
};

class Material {
public:
  uint32_t index;
};

class ModelImporter{
public:
  static Model* loadOBJ(const char* modelPath, const char* texturePath, VkBVertexBuffer* VBO, Material* mat);
};
