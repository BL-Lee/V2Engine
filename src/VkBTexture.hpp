
#include <stb_image.h>

class VkBTexture
{
  int width, height, channels;
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
public:
  void destroy();

  void createTextureImage(const char* path) ;

  static void createImage(void* pixels,
			  uint32_t width, uint32_t height,
			  VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			  VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

  
  static void createDeviceMemory(VkImage& image,
				 VkDeviceMemory& imageMemory,
				 VkMemoryPropertyFlags properties);


  static void copyStagingToImage(VkBuffer buffer,
				 VkImage image,
				 uint32_t width, uint32_t height) ;


  static void transitionImageLayout(VkImage image,
				    VkFormat format,
				    VkImageLayout oldLayout,
				    VkImageLayout newLayout);

  
  static void createDeviceImage(VkImage& image,
				uint32_t width, uint32_t height,
				VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);

  static void transferTextureToStaging(VkBuffer* stagingBuffer,
				       VkDeviceMemory* stagingBufferMemory,
				       void* pixels,
				       size_t imageSize);
};


