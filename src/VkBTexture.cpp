#include "VkBGlobals.hpp"
#include "VkBTexture.hpp"
#include "VkBBuffer.hpp"
#include "VkBSingleCommandBuffer.hpp"
#include <stdexcept>
#include <cstring>
void VkBTexture::destroy() {
  vkDestroySampler(device, textureSampler, nullptr);
  vkDestroyImageView(device, imageView, nullptr);
  vkDestroyImage(device, image, nullptr);
  vkFreeMemory(device, imageMemory, nullptr);
}


void VkBTexture::createImageView() {

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  if (depth > 1)
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
  else
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0; //Add mips later?
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }
}

void VkBTexture::setPropertiesFromType(VkBTextureType type) {
  switch (type){
  case VKB_TEXTURE_TYPE_DEPTH:
    {
      format = VK_FORMAT_D32_SFLOAT;
      tiling = VK_IMAGE_TILING_OPTIMAL;
      usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      imageSize = width * height * depth * 4;
      channels = 1;
      aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }break;
    
  case VKB_TEXTURE_TYPE_SAMPLED_RGBA:
    {
      format = VK_FORMAT_R8G8B8A8_SRGB;
      tiling = VK_IMAGE_TILING_OPTIMAL;
      usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
      properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      imageSize = width * height * depth *4;
      channels = 4;
      aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    }break;
  case VKB_TEXTURE_TYPE_STORAGE_SAMPLED_RGBA:
    {
      format = VK_FORMAT_R8G8B8A8_UNORM;
      tiling = VK_IMAGE_TILING_OPTIMAL;
      properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
	VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

      imageSize = width * height * depth *4;
      channels = 4;
      aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    }break;

  case VKB_TEXTURE_TYPE_STORAGE_RGBA:
    {
      format = VK_FORMAT_B8G8R8A8_UNORM;
      tiling = VK_IMAGE_TILING_OPTIMAL;
      usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
	VK_IMAGE_USAGE_STORAGE_BIT;

      properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      imageSize = width * height * depth * 4;
      channels = 4;
      aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    }break;

  default:
    {
      throw std::runtime_error("Invalid texture type VkBTexture.cpp");
    }break;
  }

}

void VkBTexture::createTextureImage(VkBTextureType type,
				    uint32_t w, uint32_t h,
				    void* pixels
				    ) {
  width = w;
  height = h;
  depth = 1;
  setPropertiesFromType(type);
  
  createDeviceImage();
  createDeviceMemory();

  transferPixels(pixels);
  createImageView();

  initSampler();
}

void VkBTexture::createTextureImage3D(VkBTextureType type,
				      uint32_t w, uint32_t h, uint32_t d,
				    void* pixels
				    ) {
  
  width = w;
  height = h;
  depth = d;
  setPropertiesFromType(type);
  
  createDeviceImage();
  createDeviceMemory();
  transferPixels(pixels);
  createImageView();

  initSampler();
}


void VkBTexture::transferPixels(void* pixels)
{
  if (pixels) //Copies to memory, but if null then we haven't filled it yet
    {
      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      transferTextureToStaging(&stagingBuffer, &stagingBufferMemory, pixels, imageSize);


      vkBindImageMemory(device, image, imageMemory, 0);
    
      transitionImageLayout(image,
			    format,
			    VK_IMAGE_LAYOUT_UNDEFINED,
			    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
      copyStagingToImage(stagingBuffer,
			 image,
			 width,
			 height);

      transitionImageLayout(image,
			    format,
			    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      vkDestroyBuffer(device, stagingBuffer, nullptr);
      vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
  else
    {
      vkBindImageMemory(device, image, imageMemory, 0);
      if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
	transitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
      

    }

}



void VkBTexture::createTextureImage(VkBTextureType type, const char* path) {
  stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
      
  if (!pixels) {
    throw std::runtime_error(std::string("failed to load texture image: ") + path);
  }

  createTextureImage(type, width, height, pixels);
  stbi_image_free(pixels);
}

void VkBTexture::initSampler() {

  
  VkPhysicalDeviceProperties physProperties{};
  vkGetPhysicalDeviceProperties(physicalDevice, &physProperties);
  
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_NEAREST;
  samplerInfo.minFilter = VK_FILTER_NEAREST;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_FALSE;
  //samplerInfo.maxAnisotropy = physProperties.limits.maxSamplerAnisotropy; //Max quality.. maybe lower
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE; //If for some wicked reason thy wish to address thy texture with (0, texWidth) and (0,texHeight), henceforth set this to VK_TRUE
  
  //Used for PCF on shadow maps
  samplerInfo.compareEnable = VK_FALSE; 
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
  }
}
  
void VkBTexture::createDeviceMemory()
  {
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate image memory!");
    }
  }

void VkBTexture::copyStagingToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) 
{
  VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {
    width,
    height,
    1
  };
  
  vkCmdCopyBufferToImage(
			 commandBuffer,
			 buffer,
			 image,
			 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			 1,
			 &region
			 );

  
  
  vKEndSingleTimeCommandBuffer(commandBuffer);
}

void VkBTexture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
  {
    VkCommandBuffer commandBuffer = vKBeginSingleTimeCommandBuffer();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0; //no mips and not an array 
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    // Write doesn't need to wait for anything
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      
      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      
    }
    //For storage images
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      
      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      
    }

    //Make sure it waits for the transfer to happen before reading in shader
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      
      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
      throw std::invalid_argument("unsupported layout transition!");
    }

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
  
void VkBTexture::createDeviceImage()
{
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  if (depth > 1)
    imageInfo.imageType = VK_IMAGE_TYPE_3D;
  else
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = depth;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // for multisampling
  imageInfo.flags = 0; // Optional (for sparse images)
  if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

}
  
void VkBTexture::transferTextureToStaging(VkBuffer* stagingBuffer,
					  VkDeviceMemory* stagingBufferMemory,
					  void* pixels,
					  size_t imageSize) {
  createBuffer(imageSize,
	       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	       *stagingBuffer, *stagingBufferMemory);

  void* data;
  vkMapMemory(device, *stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, imageSize);
  vkUnmapMemory(device, *stagingBufferMemory);
}
