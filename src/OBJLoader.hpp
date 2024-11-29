#pragma once
#include "VkBGlobals.hpp"
#include "VkBVertexBuffer.hpp"
#include "VkBTexture.hpp"
#include "VkBUniformBuffer.hpp"
class Model{
public:
  
  VkBTexture textures; //Just diffuse for now
  VkBUniformBuffer modelUniform; //Thisl'l need to change to 
  uint32_t vertexCount;
  uint32_t indexCount;
  uint32_t startIndex;
  uint32_t vertexOffset;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  glm::mat4 modelMatrix;

  uint32_t indexIntoModelMatrixBuffer;
  
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
