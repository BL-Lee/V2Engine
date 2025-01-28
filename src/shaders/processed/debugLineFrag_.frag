#version 450

struct Material {
  vec4 colour;
  vec2 atlasMin;
  vec2 atlasMax;
};

layout(std140, set = 2, binding = 0) readonly buffer Materials {
   Material materials[];
};

layout(set = 3, binding = 8) uniform sampler2D diffuseAtlas;
layout(set = 3, binding = 9) uniform sampler2D bumpAtlas;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 tangent;
layout(location = 4) flat in uint matIndex;

layout (location = 0) out vec4 outNormal;
layout (location = 1) out vec4 outAlbedo;
layout (location = 2) out vec4 outUV;
//layout todo material index,?

void main() 
{
  outAlbedo = vec4(1.0,0.0,1.0,1.0);
  outUV = vec4(0.0,0.0,0.5,1.0);
  outNormal = vec4(0.0,0.0,0.0,0.0);
}
