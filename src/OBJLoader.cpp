#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "OBJLoader.hpp"
#include <iostream>
Model::~Model()
{
  modelUniform.destroy();
  VBO.destroy();
  textures.destroy();
}

Model* ModelImporter::loadOBJ(const char* modelPath, const char* texturePath)
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

	vertex.texCoord = {
	  attrib.texcoords[2 * index.texcoord_index + 0],
	  1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
	};

	vertex.colour = {1.0f, 1.0f, 1.0f};

	if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
	
        indices.push_back(uniqueVertices[vertex]);
      }
    }

    model->VBO.create(vertices.size() * sizeof(Vertex),
	       indices.size() * sizeof(uint32_t));
    model->VBO.fill(vertices.data(), vertices.size(),
	     indices.data(), indices.size());
    model->VBO.transferToDevice(transientCommandPool, graphicsQueue);
    model->textures.createTextureImage(VKB_TEXTURE_TYPE_RGBA, texturePath);
    std::cout << vertices.size() << ": VERTICES" << std::endl;
    std::cout << indices.size() << ": INDICES" << std::endl;
    return model;
  }
