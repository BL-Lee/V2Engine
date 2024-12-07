#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "VkBVertexBuffer.hpp"
#include "OBJLoader.hpp"
class VkBDrawCommandBuffer{

public:
  VkCommandBuffer commandBuffer;
  
  void createCommandBuffer(VkCommandPool commandPool);
  void record(VkPipeline pipeline,
	      VkPipelineLayout pipelineLayout,
	      VkBVertexBuffer* vertexBuffer,
	      int offset,
	      int count
	      );
  void end();
  void begin(VkRenderPass renderPass,
	     VkFramebuffer frameBuffer,
	     VkExtent2D extent);

  void record(VkPipeline pipeline,
			     VkPipelineLayout pipelineLayout,
			     VkBVertexBuffer* vertexBuffer,
			     Model* model
	      );

};
