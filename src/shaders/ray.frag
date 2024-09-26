#version 450

layout(set=1, binding = 1) uniform cameraUniform {
  mat4 view;
  mat4 proj;
  float nearClip;
  float farClip;
} _MainCamera;

struct vertex {
  vec3 position;
  vec3 colour;
  vec2 texCoord;
};

layout(std140, binding = 0) readonly buffer {
   vertex vertices[ ];
};
layout (binding = 0, rgba8) uniform writeonly image2D frameBuffer;
layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

void main() {
  vec3 pixel = vec3(gl_GlobalInvocationID.xy,1.0);
  imageStore(outputImage, ivec2(gl_GlobalInvocationID.xy), pixel);
}
