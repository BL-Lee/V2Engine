#include "TextureAtlas.hpp"
#include <stdexcept>

#include "VkBSingleCommandBuffer.hpp"
#include "VkBBuffer.hpp"

void TextureAtlas::init(uint32_t w, uint32_t h, VkBTextureType type, uint32_t blockSize)
{
  //width = w;
  //height = h;

  atlas.createTextureImage(type, w, h, nullptr);    

  blockAxisCountH = h / blockSize;
  blockAxisCountW = w / blockSize;
  
  blockWidth = blockSize;
  blockHeight = blockSize;

  transitionAtlasToWrite();
  
  createBuffer(atlas.imageSize,
	       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	       stagingBuffer, stagingBufferMemory);

  vkMapMemory(device, stagingBufferMemory, 0, atlas.imageSize, 0, &stagingBufferData);

  
  occupiedCount = (int*)calloc(sizeof(int), blockAxisCountW * blockAxisCountH);
  occupiedNames = std::vector<std::string>();
  occupiedNames.resize(blockAxisCountH * blockAxisCountW);
}

void TextureAtlas::transitionAtlasToWrite()
{
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; //Should be shader read, but assuming we're only writing once
  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  barrier.image = atlas.image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

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
void TextureAtlas::transitionAtlasToSample()
{
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
  barrier.image = atlas.image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
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
// returns [-1,-1] if not available
glm::ivec2 TextureAtlas::checkIfTextureExists(std::string name)
{
   for (int i = 0; i < blockAxisCountH; i++)
    {
      for (int j = 0; j < blockAxisCountW; j++)
	{
	  if (occupiedCount[i * blockAxisCountW + j] &&
	      name == occupiedNames[i * blockAxisCountW + j])
	    {
	      occupiedCount[i * blockAxisCountW + j]++;
	      return glm::ivec2(i * blockWidth, j * blockHeight);
	    }
	}
    }
   return glm::ivec2(-1,-1);
}
glm::ivec2 TextureAtlas::requestAtlasPage()
{
  for (int i = 0; i < blockAxisCountH; i++)
    {
      for (int j = 0; j < blockAxisCountW; j++)
	{
	  if (!occupiedCount[i * blockAxisCountW + j])
	    {
	      return glm::ivec2(i * blockWidth, j * blockHeight);
	    }
	}
    }
  throw std::runtime_error("Texture atlas full");
}

void TextureAtlas::destroy()
{
  vkUnmapMemory(device, stagingBufferMemory);
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
  atlas.destroy();
}

void TextureAtlas::addToAtlas(void* pixels, glm::ivec2 position, std::string name)
{

  glm::ivec2 atlasPos = position / glm::ivec2(blockWidth, blockHeight);
  occupiedCount[atlasPos.x * blockAxisCountW + atlasPos.y]++;
  if (occupiedCount[atlasPos.x * blockAxisCountW + atlasPos.y] == 1)
    occupiedNames[atlasPos.x * blockAxisCountW + atlasPos.y] = name;
  
  memcpy(stagingBufferData, pixels, blockHeight * blockWidth  * 4); //TODO: Dont assume rgba

  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {position.x, position.y, 0};
  region.imageExtent = {
    blockWidth,
    blockHeight,
    1
  };
  
  vkCmdCopyBufferToImage(
			 commandBuffer,
			 stagingBuffer,
			 atlas.image,
			 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			 1,
			 &region
			 );

    
  vKEndSingleTimeCommandBuffer(commandBuffer);

}
