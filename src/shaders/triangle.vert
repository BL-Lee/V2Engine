#version 450

layout(set=0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    //mat4 view;
    //mat4 proj;
} ubo;

layout(set=1, binding = 1) uniform cameraUniform {
  mat4 view;
  mat4 proj;
  float nearClip;
  float farClip;
} _MainCamera;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;	
layout(location = 2) in vec2 texCoord;
layout(location = 3) in uint materialIndex;
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 worldPos;
layout(location = 2) out vec3 worldNormal;	

void main() {
    gl_Position = _MainCamera.proj * _MainCamera.view * ubo.model * vec4(position, 1.0);
    worldPos = (ubo.model * vec4(position, 1.0)).xyz;
    fragTexCoord = texCoord;
    worldNormal = (ubo.model * vec4(normal, 0.0)).xyz;
}
