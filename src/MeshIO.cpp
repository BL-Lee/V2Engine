#include "MeshIO.hpp"
void ModelIO::writeModel(std::ofstream& fileHandle, Model* model)
{
  size_t vCount = model->vertices.size();
  size_t iCount = model->indices.size();
  size_t diffuseLen = model->diffuseTexturePath.size();
  size_t ambientLen = model->ambientTexturePath.size();
  size_t bumpLen = model->bumpTexturePath.size();
  fileHandle.write((char*)&vCount, sizeof(size_t));
  fileHandle.write((char*)model->vertices.data(), sizeof(Vertex) * model->vertices.size());
  fileHandle.write((char*)&iCount, sizeof(size_t));
  fileHandle.write((char*)model->indices.data(), sizeof(uint32_t) * model->indices.size());

  fileHandle.write((char*)&diffuseLen, sizeof(size_t));
  fileHandle.write(model->diffuseTexturePath.c_str(), diffuseLen);
  fileHandle.write((char*)&ambientLen, sizeof(size_t));
  fileHandle.write(model->ambientTexturePath.c_str(), ambientLen);
  fileHandle.write((char*)&bumpLen, sizeof(size_t));
  fileHandle.write(model->bumpTexturePath.c_str(), bumpLen);

}
void ModelIO::saveModels(const char* modelPath, Model** models, uint32_t modelCount)
{
  std::ofstream fileHandle;
  fileHandle.open (modelPath,std::ios::binary);
  //fileHandle.write((char*)models[0]->vertices.data(), sizeof(Vertex) * models[0]->vertices.size());
  fileHandle.write((char*)&modelCount, sizeof(uint32_t));
  for (int i = 0; i < modelCount; i++)
    {
      writeModel(fileHandle, models[i]);
    }
  fileHandle.close();
}

void ModelIO::readModel(std::ifstream& fileHandle, Model* model)
{

  size_t vCount;
  size_t iCount;
  size_t diffuseLen;
  size_t ambientLen;
  size_t bumpLen;

  char buffer[512];
  fileHandle.read((char*)&vCount, sizeof(size_t));
  model->vertices.resize(vCount);
  fileHandle.read((char*)model->vertices.data(), sizeof(Vertex) * vCount);
  fileHandle.read((char*)&iCount, sizeof(size_t));
  model->indices.resize(vCount);
  fileHandle.read((char*)model->indices.data(), sizeof(uint32_t) * iCount);
  model->vertexCount = vCount;
  model->indexCount = iCount;

  fileHandle.read((char*)&diffuseLen, sizeof(size_t));
  //model->diffuseTexturePath.resize(diffuseLen);
  fileHandle.read(buffer, diffuseLen);
  buffer[diffuseLen] = 0;
  model->diffuseTexturePath = buffer;
  
  fileHandle.read((char*)&ambientLen, sizeof(size_t));
  //model->ambientTexturePath.resize(ambientLen);
  fileHandle.read(buffer, ambientLen);
    buffer[ambientLen] = 0;
  model->ambientTexturePath = buffer;
  //fileHandle.read(model->ambientTexturePath.c_str(), ambientLen);
  
  fileHandle.read((char*)&bumpLen, sizeof(size_t));
  fileHandle.read(buffer, bumpLen);
    buffer[bumpLen] = 0;
  model->bumpTexturePath = buffer;

  //model->bumpTexturePath.resize(ambientLen);
  //fileHandle.read(model->bumpTexturePath.c_str(), bumpLen);
}

Model** ModelIO::loadModels(const char* modelPath, Material* mat, uint32_t* modelCount)
{
  std::ifstream fileHandle;
  fileHandle.open (modelPath, std::ios::binary);

  uint32_t count;
  fileHandle.read((char*)&count, sizeof(uint32_t));
  *modelCount = count;
  Model** models = (Model**)malloc(sizeof(Model*) * count);
  for (int i = 0; i < count; i++)
    {
      models[i] = new Model();
      readModel(fileHandle, models[i]);
      models[i]->modelMatrix = glm::mat4(1.0);
    }
  fileHandle.close();
  return models;
}
