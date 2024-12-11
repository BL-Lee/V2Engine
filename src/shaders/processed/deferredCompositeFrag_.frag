#version 450

layout(set = 0, binding = 0) uniform sampler2D normals;
layout(set = 0, binding = 1) uniform sampler2D worldPos;
//layout(set = 0, binding = 2) uniform sampler2D uvs;
//layout(set = 0, binding = 3) uniform sampler2D depth;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColour;


void main() {
  outColour = vec4(texture(normals, fragTexCoord).r,
  	    texture(worldPos, fragTexCoord).g,
	    1.0,
	    1.0);
  
  return;
}
