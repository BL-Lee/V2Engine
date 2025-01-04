#pragma once
#include "VkBGlobals.hpp"
#include "VkBVertexBuffer.hpp"
#include "VkBTexture.hpp"
#include "VkBUniformBuffer.hpp"
#include "Material.hpp"
#include "mikktspace.h"



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

extern SMikkTSpaceInterface mikkTSpaceInterface;
extern SMikkTSpaceContext mikkTSpaceContext;

class MikkTSpaceTranslator
{
public:

  static void init() {
    mikkTSpaceInterface.m_getNumFaces = getNumFaces;
    mikkTSpaceInterface.m_getNumVerticesOfFace = getNumVerticesOfFace;
    mikkTSpaceInterface.m_getNormal = getNormal;
    mikkTSpaceInterface.m_getPosition = getPosition;
    mikkTSpaceInterface.m_getTexCoord = getTexCoords;
    mikkTSpaceInterface.m_setTSpaceBasic = setTSpaceBasic;

    mikkTSpaceContext.m_pInterface = &mikkTSpaceInterface;
  }

  static void calculateTangents(Model *model) {
    mikkTSpaceContext.m_pUserData = model;
    genTangSpaceDefault(&mikkTSpaceContext);
  }
  
  static int getVertexIndex(const SMikkTSpaceContext *context, int iFace, int iVert)
  {
    Model* m = (Model*)context->m_pUserData;
    int index = iFace * 3 + iVert;
    return index;
  }

  static int getNumFaces(const SMikkTSpaceContext *context)
  {
    Model* m = (Model*)context->m_pUserData;
    return (int)(m->vertices.size() / 3);
  }
  
  static int getNumVerticesOfFace(const SMikkTSpaceContext *context, int iFace)
  {
    return 3;
  }
  
  static void getPosition(const SMikkTSpaceContext *context,
			  float fvPosOut[], const int iFace, const int iVert)
  {
    Model* m = (Model*)context->m_pUserData;
    int index = iFace * 3 + iVert;
    glm::vec3 pos = m->vertices[index].pos;
    fvPosOut[0] = pos.x;
    fvPosOut[1] = pos.y;
    fvPosOut[2] = pos.z;
  }
  
  static void getNormal(const SMikkTSpaceContext *context, float fvNormOut[],
			 int iFace, int iVert)
  {
    Model* m = (Model*)context->m_pUserData;
    int index = iFace * 3 + iVert;
    glm::vec3 normal = m->vertices[index].normal;
    fvNormOut[0] = normal.x;
    fvNormOut[1] = normal.y;
    fvNormOut[2] = normal.z;

  }
  static void getTexCoords(const SMikkTSpaceContext *context, float fvTexcOut[],
			     int iFace, int iVert)
  {
    Model* m = (Model*)context->m_pUserData;
    int index = iFace * 3 + iVert;
    glm::vec2 uv = m->vertices[index].texCoord;
    fvTexcOut[0] = uv.x;
    fvTexcOut[1] = uv.y;

  }
  static void setTSpaceBasic(const SMikkTSpaceContext * context, const float fvTangent[],
			     const float fSign, const int iFace, const int iVert)
  {
    Model* m = (Model*)context->m_pUserData;
    int index = iFace * 3 + iVert;
    m->vertices[index].tangent.x = fvTangent[0];
    m->vertices[index].tangent.y = fvTangent[1];
    m->vertices[index].tangent.z = fvTangent[2];
    m->vertices[index].tangent.w = fSign;
  }

};


class ModelImporter{
public:
  static Model** loadOBJ(const char* modelPath, Material* mat, uint32_t* modelCount);
};
