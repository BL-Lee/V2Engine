#include "VkBLightProbes.hpp"
#include "VkBBuffer.hpp"
#include "VkBSingleCommandBuffer.hpp"
void VkBLightProbeInfo::create()
  {
    resolution = 32;

    cascadeCount = 4;
    
    gridDimensions = glm::vec3(2.2f,2.2f,2.2f);//Doesn't do anything yet in the shader. Is defined in the shader
    center = glm::vec3(0.0f,0.0f,0.0f);//Doesn't do anything yet in the shader
    raysPerProbe = 20; //Same

    imageWidth = resolution * 2;
    //DEBUG LINES
    /*
    VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    createBuffer(imageWidth * imageWidth * imageWidth *sizeof(uint8_t) * 4,
		 usageFlags,
		 memFlags,
		 lineBuffer,
		 lineBufferMemory);
    vkMapMemory(device, lineBufferMemory,
		0,
		imageWidth * imageWidth * imageWidth * sizeof(uint8_t) * 4,
		0, &lineBufferMapped);
    */

    
    debugCascadeViewIndex = 0;
    debugDirectionViewIndex = 0;
    viewDebug = 0;
    lineCount = imageWidth * imageWidth * imageWidth;
    lines = (LineVertex*)malloc(2 * lineCount * sizeof(LineVertex));
    lineVBO.create(lineCount * 2 * sizeof(LineVertex));
  }

void VkBLightProbeInfo::transitionImageToSampled(VkImage image)
{
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
    
  VkPipelineStageFlags sourceStage =  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(
		       commandBuffer,
		       sourceStage, destinationStage,
		       0,
		       0, nullptr,
		       0, nullptr,
		       1, &barrier
		       );

  vKEndSingleTimeCommandBuffer(commandBuffer);

}
void VkBLightProbeInfo::transitionImageToStorage(VkImage image)
{
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
    
  VkPipelineStageFlags sourceStage =  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(
		       commandBuffer,
		       sourceStage, destinationStage,
		       0,
		       0, nullptr,
		       0, nullptr,
		       1, &barrier
		       );

    
  vKEndSingleTimeCommandBuffer(commandBuffer);
}
/*
void VkBLightProbeInfo::processLightProbeTextureToLines()
{
  //TEMP DIRECTIONS
  glm::vec3 dirs[16] = {

    glm::normalize(glm::vec3(0.0, 1.0, 0.0)),
    glm::normalize(glm::vec3(0.0, -1.0, 0.0)),
    glm::normalize(glm::vec3(0.0, 0.0, 1.0)),
    glm::normalize(glm::vec3(0.0, 0.0, -1.0)),
    
    glm::normalize(glm::vec3(1.0, 0.0, 0.0)),
    glm::normalize(glm::vec3(-1.0, 0.0, 0.0)),

    glm::normalize(glm::vec3(-1.0, -1.0, -1.0)),
    glm::normalize(glm::vec3(1.0, -1.0, -1.0)),
    glm::normalize(glm::vec3(-1.0, 1.0, -1.0)),
    glm::normalize(glm::vec3(1.0, 1.0, -1.0)),

    glm::normalize(glm::vec3(-1.0, -1.0, 1.0)),
    glm::normalize(glm::vec3(1.0, -1.0, 1.0)),
    glm::normalize(glm::vec3(-1.0, 1.0, 1.0)),
    glm::normalize(glm::vec3(1.0, 1.0, 1.0)),
    glm::normalize(glm::vec3(-1.0, 0.0, 1.0)),
    glm::normalize(glm::vec3(1.0, 0.0, 1.0))
  };

  int lineIndex = 0;
  for (int x = 0; x < imageWidth; x++)
    {
      for (int y = 0; y < imageWidth; y++)
	{
	  uint8_t r = ((uint8_t*)lineBufferMapped)[(x * imageWidth + y) * 4 + 0];
	  uint8_t g = ((uint8_t*)lineBufferMapped)[(x * imageWidth + y) * 4 + 1];
	  uint8_t b = ((uint8_t*)lineBufferMapped)[(x * imageWidth + y) * 4 + 2];
	  
	  glm::vec3 pixel = {
	    (float)r / 255,
	    (float)g / 255,
	    (float)b / 255,
	  };

	  int quadrantCount = 2; //TODO: unhardcode
	  //get quadrant
	  glm::ivec3 dirTile = {
	    int((float)x / imageWidth * quadrantCount),
	    int((float)y / imageWidth * quadrantCount),
	    0
	  };
	  int dirIndex = dirTile.x + dirTile.y * quadrantCount;

	  //where we are inside this quadrant
	  glm::ivec3 localInvocation = {
	    x - (dirTile.x * imageWidth / quadrantCount),
	    y - (dirTile.y * imageWidth / quadrantCount),
	    0
	  };
	  int gridSize = 64;
	  int tilingCount = 2; 
	  glm::vec3 invocationCoord =
	    {
	      localInvocation.x % gridSize,
	      (localInvocation.x / gridSize) +
	      (localInvocation.y / gridSize) * tilingCount,
	      localInvocation.y % gridSize
	    };

	  
	  glm::vec3 position = invocationCoord / (float)gridSize;
	  position -= 0.5f;
	  position *= gridDimensions;
	  position += center;
	  glm::vec3 direction = dirs[dirIndex];

	  lines[lineIndex + 0].pos = position;
	  lines[lineIndex + 0].colour = pixel;
	  lines[lineIndex + 1].pos = position + direction / 10.0f;
	  lines[lineIndex + 1].colour = pixel;
	  lineIndex += 2;
	}
    }
  uint32_t a;
  lineVBO.fill(lines, lineCount * 2,  &a);
  
}

void VkBLightProbeInfo::copyTextureToCPU(VkBTexture* tex)
{
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();

  VkOffset3D imageOffset{};
  imageOffset.x = 0;   imageOffset.y = 0;   imageOffset.z = debugCascadeViewIndex;

  VkExtent3D imageExtent{};
  imageExtent.width = imageWidth;
  imageExtent.height = imageWidth;
  imageExtent.depth = 1;

  VkImageSubresourceLayers imageSubresource{};
  imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageSubresource.mipLevel = 0;
  imageSubresource.baseArrayLayer = 0;
  imageSubresource.layerCount = 1;
  
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource = imageSubresource;
  region.imageOffset = imageOffset;
  region.imageExtent = imageExtent;
  
  
  vkCmdCopyImageToBuffer(
    commandBuffer,
    tex->image,
    VK_IMAGE_LAYOUT_GENERAL,
    lineBuffer,
    1,
    &region);

  vKEndSingleTimeCommandBuffer(commandBuffer);
  }*/
void VkBLightProbeInfo::destroy() {
  for (int i = 0; i < cascadeCount; i++)
    textures[i].destroy();

  //vkDestroyBuffer(device, lineBuffer, nullptr);
  //vkFreeMemory(device, lineBufferMemory, nullptr);
  lineVBO.destroy();
  free(lines);

}
