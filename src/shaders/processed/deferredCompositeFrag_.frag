#version 450


vec3 worldPosFromDepth(float depth, vec2 screenPos, mat4 invProjView)
{
  vec4 clipPos;
  clipPos.xy = screenPos * 2.0 - 1.0;
  clipPos.z = depth;
  clipPos.w = 1.0;

  vec4 homoCoord = invProjView * clipPos;
  return homoCoord.xyz / homoCoord.w;
}
layout(set = 0, binding = 0) uniform sampler2D normals;
layout(set = 0, binding = 1) uniform sampler2D albedos;
layout(set = 0, binding = 2) uniform sampler2D depth;
//layout(set = 0, binding = 3) uniform sampler2D depth;

layout(set = 1, binding = 2) uniform sampler2D probeSamplers[5];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColour;

vec3 initialDirs[4] = {
  vec3( 1, 1, 1),
  vec3(-1, 1,-1),
  vec3(-1,-1, 1),
  vec3( 1,-1,-1),
};

layout( push_constant ) uniform cascadeConstant {
  int cascade;
  int quadrant;
  float bilateralBlend;
  float start;
  float end;
  int lineView;
} cascadeInfo;

layout(set = 2, binding = 1) uniform cameraUniform {
  mat4 view;
  mat4 proj;
  mat4 invViewProj;
  float width;
  float height;
  float nearClip;
  float farClip;
} _MainCamera;



vec4 toSRGB(vec4 i)
{
  return pow(i, vec4(2.2,2.2,2.2,1.0));
}

void main() {
  vec3 col = texture(albedos, fragTexCoord).xyz;
  //float depth = texture(depth, fragTexCoord).r;
  //vec3 worldPos = worldPosFromDepth(depth, fragTexCoord, _MainCamera.invViewProj);

  //  vec3 worldPos = val.rgb;
  /*
  int matIndex = int(round(val.a));

  if (matIndex == 0)

    {
      vec4 emitColour = vec4(1.0,0.9,0.4,1.0) * 10.0;
      outColour = emitColour;
      return;
    }

  vec4 albedo = vec4(0.0);
  if (matIndex == 1) //diffuseGrey
      albedo = vec4(0.5,0.5,0.5,1.0) * 0.2;
  if (matIndex == 2) //diffuseGrey
    albedo = vec4(1.0,0.0,0.0,1.0) * 0.2;
  if (matIndex == 3) //diffuseGrey
    albedo = vec4(0.0,1.0,0.0,1.0) * 0.2;
  //if (matIndex == 4) //reflective
  else
    albedo = vec4(1.0,1.0,0.0,1.0);
    
  albedo.a = 1.0;
  */
  outColour = vec4(col,1.0);
   return ;
  
  vec3 width = vec3(2.5);
  vec3 center = vec3(0.0,1.0,0.0);
  
  vec4 radiance = vec4(0.0);
  
  int cascade = cascadeInfo.cascade;
  //Debug show outside of range
  
  int dirCount = int(pow(4, cascade));
  int dirTilingCount = int(pow(2, cascade));

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
  radiance *= 4.0; //Hlaf of rays point inwards?
  //radiance.a = 1.0;
  //  outColour = radiance * albedo;
  return;

  //outColour = toSRGB(radiance);

  
  //return;
}
