#pragma once
#include "VkBGlobals.hpp"
#include "VkBVertexBuffer.hpp"
#include "VkBTexture.hpp"
#include "VkBUniformBuffer.hpp"
#include "Material.hpp"
class Model{
public:
  uint32_t vertexCount;
  uint32_t indexCount;
  uint32_t startIndex;
  uint32_t vertexOffset;

  std::vector<Vertex> vertices; //Should clear after we;re done with it? lives only on gpu
  std::vector<uint32_t> indices;
  glm::mat4 modelMatrix;
  uint32_t indexIntoModelMatrixBuffer;
  uint32_t indexIntoMaterialBuffer;
  
  std::string diffuseTexturePath;
  std::string ambientTexturePath;
  std::string bumpTexturePath;

  Material material;
  void* rootBVHNode;
  void setVerticesMatIndex();
  void normalizeUVs();
  void addToVBO(VkBVertexBuffer* vbo);
  
  ~Model();
};

class ModelImporter{
public:
  static Model** loadOBJ(const char* modelPath, Material* mat, uint32_t* modelCount);
};
