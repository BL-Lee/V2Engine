#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <array>
#include "glm/glm.hpp"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex
{
  //std140 vulkan requirements for storage buffer
  alignas(sizeof(glm::vec4)) glm::vec3 pos;
  alignas(sizeof(glm::vec4)) glm::vec3 normal;
  alignas(sizeof(glm::vec4)) glm::vec4 tangent;
  alignas(sizeof(glm::vec2)) glm::vec2 texCoord;
  alignas(sizeof(float)) uint32_t materialIndex;

  bool operator==(const Vertex& other) const {
    return pos == other.pos && texCoord == other.texCoord;
  }

  
  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};

    //This will need to change when i use multiple Vertex arrays
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return bindingDescription;
  }

  //So this needs to change if we're having different properties in the vertex
  //Such as normal/texcoord
  static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);
    
    attributeDescriptions[1].binding = 0; //Something about instances
    attributeDescriptions[1].location = 1; //Where in the shader it goes into
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0; //Something about instances
    attributeDescriptions[2].location = 2; //Where in the shader it goes into
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, tangent);

    attributeDescriptions[3].binding = 0; //Something about instances
    attributeDescriptions[3].location = 3; //Where in the shader it goes into
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, texCoord);
    
    attributeDescriptions[4].binding = 0; //Something about instances
    attributeDescriptions[4].location = 4; //Where in the shader it goes into
    attributeDescriptions[4].format = VK_FORMAT_R32_UINT;
    attributeDescriptions[4].offset = offsetof(Vertex, materialIndex);

    
    return attributeDescriptions;
  }
};

namespace std {
  template<> struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const {
      return ((hash<glm::vec3>()(vertex.pos) ^
	       (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
	(hash<glm::vec2>()(vertex.texCoord) << 1);
    }
  };
}


struct LineVertex
{
  //std140 vulkan requirements for storage buffer
  alignas(sizeof(glm::vec4)) glm::vec3 pos;
  alignas(sizeof(glm::vec4)) glm::vec3 colour;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};

    //This will need to change when i use multiple Vertex arrays
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(LineVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return bindingDescription;
  }

  //So this needs to change if we're having different properties in the vertex
  //Such as normal/texcoord
  static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(LineVertex, pos);

    
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(LineVertex, colour);

    return attributeDescriptions;
  }
};

