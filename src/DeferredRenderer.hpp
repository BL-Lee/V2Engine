#pragma once
#include "VkBGlobals.hpp"
#include "VkBDrawCommandBuffer.hpp"
#include "VkBGraphicsPipeline.hpp"
#include "VkBRenderPass.hpp"
#include "VkBVertexBuffer.hpp"
#include "VkBTexture.hpp"
#include "VkBUniformPool.hpp"
#include "VkBUniformBuffer.hpp"
#include "OBJLoader.hpp"
#include <vector>
class DeferredRenderer
{
public:
  VkBDrawCommandBuffer drawCommandBuffer;
  VkBDrawCommandBuffer compositeCommandBuffer;
  
  VkBGraphicsPipeline deferredPipeline;
  VkFramebuffer deferredFramebuffer;
  VkBRenderPass deferredRenderPass;
  
  VkBGraphicsPipeline compositePipeline;
  VkFramebuffer compositeFramebuffer;
  VkBRenderPass compositeRenderPass;

  VkBUniformPool compositeUniformPool;
  VkBUniformBuffer compositeUniform;

  VkBTexture* deferredTextures;
  
  uint32_t mode = 0; //0: deferred, 1: composite
  VkBVertexBuffer fullscreenQuad;

  void init(VkBSwapChain* swapChain);
  void destroy();

  void setCompositeInformation(VkBGraphicsPipeline pipeline,
			       VkBRenderPass renderPass,
			       VkFramebuffer framebuffer );

  void begin( VkExtent2D extent );
  void bindDescriptorSet( VkDescriptorSet* dSet, uint32_t ind );
  void bindDescriptorSetComposite( VkDescriptorSet* dSet, uint32_t ind );
  void record(VkBVertexBuffer* vertexBuffer, Model* model);

  void record(VkBVertexBuffer* vertexBuffer, uint32_t start, uint32_t stop);

  void submitDeferred(  VkSubmitInfo* submitInfo, VkFence fence);
  void submitComposite(  VkSubmitInfo* submitInfo, VkFence fence, VkExtent2D);
  void changeDeferredPipeline(VkBGraphicsPipeline pipeline);
  void changeCompositePipeline(VkBGraphicsPipeline pipeline);
};
