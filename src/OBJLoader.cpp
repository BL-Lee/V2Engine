#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "OBJLoader.hpp"
#include <iostream>

Model::~Model()
{
  modelUniform.destroy();
  textures.destroy();
}

Model* ModelImporter::loadOBJ(const char* modelPath, const char* texturePath,
			      VkBVertexBuffer* VBO,
			      Material* mat)
  {
    Model* model = new Model();
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
  
    if (!tinyobj::LoadObj(&attrib,
			  &shapes,
			  &materials,
			  &warn, &err,
			  modelPath)) {
      throw std::runtime_error(warn + err);
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes) {
      for (const auto& index : shape.mesh.indices) {
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

	vertex.materialIndex = mat->index;
	/*
	if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
	
        indices.push_back(uniqueVertices[vertex]);
	*/
	vertices.push_back(vertex);
	indices.push_back(indices.size());
      }
    }
    model->indexCount = (uint32_t)indices.size();
    model->vertexCount = (uint32_t)vertices.size();
    /*VBO.create(vertices.size() * sizeof(Vertex),
      indices.size() * sizeof(uint32_t));*/
    VBO->fill(vertices.data(), vertices.size(),
	      indices.data(), indices.size(),
	      &model->startIndex,
	      &model->vertexOffset);
    //    model->VBO.transferToDevice(transientCommandPool, graphicsQueue);
    model->textures.createTextureImage(VKB_TEXTURE_TYPE_SAMPLED_RGBA, texturePath);
    std::cout << vertices.size() << ": VERTICES" << std::endl;
    std::cout << indices.size() << ": INDICES" << std::endl;
    return model;
  }
