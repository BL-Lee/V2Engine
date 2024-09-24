#pragma once
#include "VkBGlobals.hpp"
#include "VkBVertexBuffer.hpp"
#include "VkBTexture.hpp"
#include "VkBUniformBuffer.hpp"
class Model{
public:
  VkBVertexBuffer VBO;
  VkBTexture textures; //Just diffuse for now
  VkBUniformBuffer modelUniform;
  ~Model();
};

class ModelImporter{
public:
  static Model* loadOBJ(const char* modelPath, const char* texturePath);
};
