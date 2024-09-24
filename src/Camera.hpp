#pragma once

class Camera
{
public:
  //Keep these together
  glm::mat4 view; 
  glm::mat4 projection;
  float nearClip, farClip;
  
  float fov;
  float aspectRatio;

  
  glm::vec3 position;
  glm::vec3 direction;

  
  VkBUniformBuffer ubo;

  void init()
  {
    position = glm::vec3(0.0, 0.0, 0.0);
    direction = glm::vec3(0.0, 0.0, 1.0);
    fov = 45.0f;
    nearClip = 0.01f;
    farClip = 1000.0f;
    
  }
  void createPerspective(float width, float height)
  {
    aspectRatio = width / height;
    init();
    projection = glm::perspective(glm::radians(fov),
				aspectRatio,
				nearClip, farClip);
    projection[1][1] *= -1;
  }
  void updateMatrices(uint32_t imageIndex)
  {
    view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
			   glm::vec3(0.0f, 0.0f, 0.0f),
			   glm::vec3(0.0f, 0.0f, 1.0f));
    
    //Since theyre next to each other in the class, can memcpy from view    
    memcpy(ubo.getBufferMemoryLocation(imageIndex,0), &view, sizeof(glm::mat4)*2 + sizeof(float)*2);
  }

};
