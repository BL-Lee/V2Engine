#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "OBJLoader.hpp"
#include <iostream>

Model::~Model()
{
}

Model** ModelImporter::loadOBJ(const char* modelPath,
			       Material* mat,
			       uint32_t* modelCount)
  {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
  
    if (!tinyobj::LoadObj(&attrib,
			  &shapes,
			  &materials,
			  &warn, &err,
			  modelPath,
			  "../models/")) {
      throw std::runtime_error(warn + err);
    }
    Model** models = (Model**)calloc(shapes.size(), sizeof(Model*));
    //std::vector<Vertex> vertices;
    //std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    //material_t->ambient_texname
    //material_t->bump_texname
    uint32_t shapeCount = shapes.size();
    for (int i = 0; i < shapeCount; i++)
      {
	Model* model = new Model();
	models[i] = model;
	for (const auto& index : shapes[i].mesh.indices) {
	  Vertex vertex{};

	  vertex.pos = {
	    attrib.vertices[3 * index.vertex_index + 0],
	    attrib.vertices[3 * index.vertex_index + 1],
	    attrib.vertices[3 * index.vertex_index + 2]
	  };

	  vertex.normal = {
	    attrib.normals[3 * index.normal_index + 0],
	    attrib.normals[3 * index.normal_index + 1],
	    attrib.normals[3 * index.normal_index + 2]
	  };

	  vertex.texCoord = {
	    attrib.texcoords[2 * index.texcoord_index + 0],
	    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
	  };

	  vertex.materialIndex = 0;
	  /*
	    if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
	    }
	
	    indices.push_back(uniqueVertices[vertex]);
	  */
	  model->vertices.push_back(vertex);
	  model->indices.push_back((uint32_t)model->indices.size());
	}
	model->indexCount = (uint32_t)model->indices.size();
	model->vertexCount = (uint32_t)model->vertices.size();

	model->modelMatrix = glm::mat4(1.0);
    
	//    model->VBO.transferToDevice(transientCommandPool, graphicsQueue);
	//model->textures.createTextureImage(VKB_TEXTURE_TYPE_SAMPLED_RGBA, texturePath);
	//std::cout << model->vertices.size() << ": VERTICES" << std::endl;
	//std::cout << model->indices.size() << ": INDICES" << std::endl;
	//std::cout << materials[shapes[i].mesh.material_ids[0]].diffuse_texname << std::endl;
	model->diffuseTexturePath = materials[shapes[i].mesh.material_ids[0]].diffuse_texname;
	model->ambientTexturePath = materials[shapes[i].mesh.material_ids[0]].ambient_texname;
      }

    *modelCount = shapeCount;
    
    return models;
  }
void Model::addToVBO(VkBVertexBuffer* vbo)
{
  vbo->fill(vertices.data(), (uint32_t)vertices.size(),
	    indices.data(), (uint32_t)indices.size(),
	    &startIndex,
	    &vertexOffset);
}

void Model::normalizeUVs()
{
  glm::vec2 minUV = {FLT_MAX, FLT_MAX};
  glm::vec2 maxUV = {-FLT_MAX, -FLT_MAX};

  for (int i = 0; i < vertices.size(); i++)
    {
      minUV = glm::min(minUV, vertices[i].texCoord);
      maxUV = glm::max(maxUV, vertices[i].texCoord);
    }
  for (int i = 0; i < vertices.size(); i++)
    {
      vertices[i].texCoord = (vertices[i].texCoord / (maxUV - minUV));// + minUV;
    }
}

void Model::setVerticesMatIndex()
{
  for (int i = 0; i < vertices.size(); i++)
    {
      vertices[i].materialIndex = indexIntoMaterialBuffer;
    }
}
