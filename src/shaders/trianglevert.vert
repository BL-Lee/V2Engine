#version 450

layout(std140, set = 0, binding = 4) readonly buffer ModelMatrices {
   mat4 modelMatrices[];
};

layout(set=1, binding = 1) uniform cameraUniform {
  mat4 view;
  mat4 proj;
  mat4 invViewProj;
  float width;
  float height;
  float nearClip;
  float farClip;
} _MainCamera;

layout( push_constant ) uniform transformInfo {
  layout(offset=24)
  uint matIndex;
} info;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;	
layout(location = 3) in vec2 texCoord;
layout(location = 4) in uint materialIndex;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 worldPos;
layout(location = 2) out vec3 worldNormal;
//layout(location = 3) out vec3 TEMP_EYE_POS;
layout(location = 3) out vec3 colour;	

void main() {
  mat4 modelMat = modelMatrices[info.matIndex];
  gl_Position = _MainCamera.proj * _MainCamera.view * modelMat * vec4(position, 1.0);
  worldPos = (modelMat * vec4(position, 1.0)).xyz;
  fragTexCoord = texCoord;
  worldNormal = (modelMat * vec4(normal, 0.0)).xyz;
  //    TEMP_EYE_POS = (_MainCamera.invViewProj * vec4(0.0,0.0,0.0,1.0)).xyz;

  if (materialIndex == 0) //emissive
    {
      colour = vec3(1.0,0.9,0.6);
    }
  else if (materialIndex == 1) //diffuseGrey
    {
      colour = vec3(0.5,0.5,0.5);
    }
  else if (materialIndex == 2) //red
    {
      colour = vec3(1.0,0.0,0.0);
    }
  else if (materialIndex == 3) //green
    {
      colour = vec3(0.0,1.0,0.0);
    }

}
