#version 450

layout(std140, set = 0, binding = 4) readonly buffer ModelMatrices {
   mat4 modelMatrices[];
};

layout(set=1, binding = 1) uniform cameraUniform {
  mat4 view;
  mat4 proj;
  mat4 invProj;		
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
layout(location = 2) in vec4 tangent;	
layout(location = 3) in vec2 texCoord;
layout(location = 4) in uint materialIndex;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outWorldPos;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out uint outMatIndex;

void main() {
  mat4 modelMat = modelMatrices[info.matIndex];
  gl_Position = _MainCamera.proj * _MainCamera.view * modelMat * vec4(position, 1.0);
  outWorldPos = (modelMat * vec4(position, 1.0)).xyz;
  outTexCoord = texCoord;
  
  outNormal = normal;//(modelMat * vec4(normal, 0.0)).xyz;
  outMatIndex = materialIndex;
}
