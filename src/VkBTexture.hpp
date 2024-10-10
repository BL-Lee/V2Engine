#pragma once
#include <stb_image.h>

typedef uint32_t VkBTextureType;
#define VKB_TEXTURE_TYPE_DEPTH 0x1u
#define VKB_TEXTURE_TYPE_SAMPLED_RGBA 0x2u
#define VKB_TEXTURE_TYPE_STORAGE_RGBA 0x3u

class VkBTexture
{
public:

  int width, height, depth, channels;
  VkImage image;
  VkDeviceMemory imageMemory;
  VkImageView imageView; //How you access the image, rather than going through byte offsets from the raw image
  VkSampler textureSampler;

  VkFormat format;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkMemoryPropertyFlags properties;
  VkImageAspectFlags aspectFlags;
  VkDeviceSize imageSize;

  void createTextureImage(VkBTextureType type,
			  uint32_t w, uint32_t h,
			  void* pixels
			  );
  void createTextureImage3D(VkBTextureType type,
			    uint32_t w, uint32_t h, uint32_t d,
			  void* pixels
			  );
  void transferPixels(void* pixels);
  void createTextureImage(VkBTextureType type, const char* path);

  void destroy();
  void setPropertiesFromType(VkBTextureType type);
  void initSampler();
  void createImageView();

  void createDeviceMemory();


  static void copyStagingToImage(VkBuffer buffer,
				 VkImage image,
				 uint32_t width, uint32_t height) ;


  static void transitionImageLayout(VkImage image,
				    VkFormat format,
				    VkImageLayout oldLayout,
				    VkImageLayout newLayout);

  
  void createDeviceImage();

  static void transferTextureToStaging(VkBuffer* stagingBuffer,
				       VkDeviceMemory* stagingBufferMemory,
				       void* pixels,
				       size_t imageSize);
};


