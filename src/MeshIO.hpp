#pragma once
#include "VkBGlobals.hpp"
#include "Vertex.hpp"
#include "Material.hpp"
#include "VkBVertexBuffer.hpp"
#include <fstream>
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
  void calculateTangents();
  void addToVBO(VkBVertexBuffer* vbo);
  
  ~Model();
};

class ModelIO
{
  static void writeModel(std::ofstream& fileHandle, Model* model);
  static void readModel(std::ifstream& fileHandle, Model* model);
public:
  static Model** loadModels(const char* modelPath, Material* mat, uint32_t* modelCount);
  static void saveModels(const char* modelPath, Model** models, uint32_t modelCount);
};
