#version 450

layout(set=0, binding = 1) uniform sampler2D texSampler;
layout(set=2, binding = 1) uniform sampler3D probeSampler;
layout(set = 2, binding = 0, rgba8) uniform readonly image3D probeInfo;	
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 0) out vec4 outColor;

void main() {
  vec3 width = vec3(2.2);
  vec3 center = vec3(0.0,1.0,0.0);
  vec3 texWidth = textureSize(probeSampler,0);
  //  vec3 probeTexLocation = (worldPos - (texWidth / 2.0));
  // probeTexLocation /= width;

  //need -1:1 -> 0:1
  vec3 probeTexLocation = (worldPos - center) / width;
  probeTexLocation += 0.5;

  outColor = texture(texSampler, fragTexCoord) * vec4((texture(probeSampler, probeTexLocation).rgb + 0.1),1.0);


}
