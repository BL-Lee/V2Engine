#version 450

layout(set=0, binding = 1) uniform sampler2D texSampler;
layout(set=2, binding = 1) uniform sampler3D probeSampler;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 0) out vec4 outColor;

void main() {
  vec3 probeTexLocation = worldPos + textureSize(probeSampler, 0) / 2.0;
  //outColor = texture(probeSampler, (worldPos / textureSize(probeSampler,0)) );
  outColor = texture(probeSampler, (worldPos) );
    // * imageLoad(probeSampler, ivec3(worldPos / 8.0 * imageSize(probeSampler)));
}
