#version 450

//layout(set=0, binding = 1) uniform sampler2D texSampler;

//layout(set=2, binding = 1) uniform sampler3D probeSampler;
//layout(set = 2, binding = 0, rgba8) uniform restrict readonly image3D probeInfo[4];
//layout(set = 2, binding = 2) uniform sampler3D probeSamplers[4];
//layout(set = 2, binding = 0, rgba8) uniform restrict readonly image3D probeInfo;
layout(set = 2, binding = 2) uniform sampler3D probeSamplers[4];

//layout(set = 1, binding = 0, rgba8) uniform readonly image3D probeInfo[2];
//layout(set = 2, binding = 1, rgba8) uniform readonly image3D probeInfo2;	
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 worldNormal;
//layout(location = 3) in vec3 TEMP_EYE_POS;
layout(location = 3) in vec3 colour;
layout(location = 0) out vec4 outColour;

layout( push_constant ) uniform cascadeConstant {
  layout(offset=0)
  int cascade;
  int quadrant;
  float start;
  float end;
  int lineView;
} cascadeInfo;

float pi = 3.14159;
float phi = pi * (sqrt(5.0) - 1.0);

vec3 initialDirs[8] = {
  vec3(-1,-1,-1),
  vec3( 1,-1,-1),
  vec3(-1, 1,-1),
  vec3( 1, 1,-1),
  vec3(-1,-1, 1),
  vec3( 1,-1, 1),
  vec3(-1, 1, 1),
  vec3( 1, 1, 1)
};


int getIndexFromDir(vec3 dir, int cascadeIndex)
{
  vec3 contributedDir = vec3(0.0);
  int totalIdx = 0;
  for (int i = 0; i < cascadeIndex + 1; i++)
    {
      int idx = 0;
      int totalDirs = int(pow(8, i+1));
      if (dir.z >= contributedDir.z)
	{
	  idx += 4;
	  totalIdx += 4 * totalDirs / 8;
	}
      if (dir.y >= contributedDir.y)
	{
	  idx += 2;
	  totalIdx += 2 * totalDirs / 8;
	}
      if (dir.x >= contributedDir.x)
	{
	  idx += 1;
	  totalIdx += 1 * totalDirs / 8;
	}

      contributedDir += initialDirs[idx];
      contributedDir = normalize(contributedDir);
    }
  return totalIdx;
}


void main() {

  outColour = vec4(colour,1.0);
  return;

  vec3 width = vec3(2.5);
  vec3 center = vec3(0.0,1.0,0.0);
  

  
  //need -1:1 -> 0:1
  vec3 probeTexLocation = (worldPos - center) / width;
  probeTexLocation += 0.5;
  //have 0-1
  //Now 0-1 to divided for cascades

  vec4 radiance = vec4(0.0);
  //  vec3 incidentRay = normalize(worldPos - TEMP_EYE_POS);
  //  vec3 reflectedRay = normalize(incidentRay - 2*worldNormal*(dot(incidentRay, worldNormal)));
  for (int cascade = cascadeInfo.cascade; cascade < cascadeInfo.cascade + 1; cascade++)
  //for (int cascade = 0; cascade < 4; cascade++)
  //for (int cascade = 0; cascade < 1; cascade++) //If we're accumulating down the cascade chain.. only need to sample lowest
    {
      //Debug show outside of range
      vec3 pos = worldPos - center;
      if (pos.x > width.x/2 ||
	  pos.x <-width.x/2 ||
	  pos.y > width.y/2 ||
	  pos.y <-width.y/2 ||
	  pos.z > width.z/2 ||
	  pos.z <-width.z/2)
	{
	  radiance = vec4(0.0,1.0,1.0,1.0);
	  break;
	}
  

      vec3 normalizedCoords = probeTexLocation;
      int dirCount = int(pow(8, cascade + 1)); //TODO unhardcode
      int dirTilingCount = int(pow(2, cascade + 1)); //TODO cube root
      
      //scale down to quadrant 
      vec3 quadrantLocalCoord = probeTexLocation / vec3(dirTilingCount, dirTilingCount, dirTilingCount);

      vec3 coord = vec3(quadrantLocalCoord * textureSize(probeSamplers[cascade],0));
      //int dir = getIndexFromDir(normalize(vec3(1.0,0.0,0.0)), cascade);
      for (int dir = 0; dir < dirCount; dir++)
        {
          ivec3 quadrantOffset = ivec3(dir % dirTilingCount,
				       (dir / dirTilingCount) % dirTilingCount,
				       dir / (dirTilingCount * dirTilingCount)) * textureSize(probeSamplers[cascade],0) / dirTilingCount;
	  vec3 texCoord = vec3(coord + quadrantOffset) / textureSize(probeSamplers[cascade],0);
	  vec4 val = texture(probeSamplers[cascade], texCoord);
	  radiance += val / dirCount;
        }
    }
  radiance /= radiance.a;
  //radiance.a = 1.0;
  //outColor = radiance;
  //return;
  outColour = vec4(colour,1.0) * (radiance);
}
