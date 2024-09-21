#pragma once
#include "VkBGlobals.hpp"
#include "VkBVertexBuffer.hpp"
#include "VkBTexture.hpp"
class Model{
public:
  VkBVertexBuffer VBO;
  VkBTexture textures; //Just diffuse for now
  void destroy();
};

class ModelImporter{
public:
  static Model* loadOBJ(const char* modelPath, const char* texturePath);
};
