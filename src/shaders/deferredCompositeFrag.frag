#version 450

layout(set = 0, binding = 0) uniform sampler2D normals;
layout(set = 0, binding = 1) uniform sampler2D worldPos;
layout(set = 0, binding = 2) uniform sampler2D uvs;
//layout(set = 0, binding = 3) uniform sampler2D depth;

layout(set = 1, binding = 2) uniform sampler2D probeSamplers[4];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColour;

vec3 initialDirs[4] = {
  vec3( 1, 1, 1),
  vec3(-1, 1,-1),
  vec3(-1,-1, 1),
  vec3( 1,-1,-1),
};


vec4 toSRGB(vec4 i)
{
  return pow(i, vec4(2.2,2.2,2.2,1.0));
}

void main() {
  vec4 val = texture(worldPos, fragTexCoord);
  vec3 worldPos = val.rgb;
  float matIndex = val.a;

  if (matIndex == 0)
    {
      vec4 emitColour = vec4(1.0,0.9,0.4,1.0) * 10.0;
      outColour = emitColour;
      return;
    }

  vec4 albedo = vec4(0.0);
  if (matIndex == 1) //diffuseGrey
      albedo = vec4(0.5,0.5,0.5,1.0) * 0.2;
  else if (matIndex == 2) //red
      albedo = vec4(1.0,0.0,0.0,1.0) * 0.2;
  else if (matIndex == 3) //green
      albedo = vec4(0.0,1.0,0.0,1.0) * 0.2;
  albedo.a = 1.0;

  vec3 width = vec3(2.5);
  vec3 center = vec3(0.0,1.0,0.0);
  
  vec4 radiance = vec4(0.0);
  
  int cascade = 0;
  //Debug show outside of range
  
  int dirCount = int(pow(4, cascade + 1));
  int dirTilingCount = int(pow(2, cascade + 1));

  //vec2 texCoord = quadrantLocalCoord + quadrantOffset;
  //radiance = texture(probeSamplers[cascade], fragTexCoord);
  //radiance += val / dirCount;


  //scale down to quadrant 
  vec2 quadrantLocalCoord = fragTexCoord / vec2(dirTilingCount);
  //vec3 coord = vec3(quadrantLocalCoord * textureSize(probeSamplers[cascade],0));
  for (int dir = 0; dir < dirCount; dir++)
    //for (int dir = 0; dir < 1; dir++)
    {
      vec2 quadrantOffset = vec2(dir % dirTilingCount,
				   dir / dirTilingCount) / dirTilingCount;
      vec2 texCoord = quadrantLocalCoord + quadrantOffset;
      vec4 val = texture(probeSamplers[cascade], texCoord);
      radiance += val / dirCount;

    }

  radiance /= radiance.a;
  //radiance.a = 1.0;
  outColour = radiance;
  return;

  outColour = toSRGB(radiance);

  
  return;
}
