#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <array>
#include "glm/glm.hpp"
struct Vertex
{
  glm::vec3 pos;
  glm::vec3 colour;
  glm::vec2 texCoord;

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
  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);
    
    attributeDescriptions[1].binding = 0; //Something about instances
    attributeDescriptions[1].location = 1; //Where in the shader it goes into
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, colour);

    attributeDescriptions[2].binding = 0; //Something about instances
    attributeDescriptions[2].location = 2; //Where in the shader it goes into
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    
    return attributeDescriptions;
  }
};
