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
vec3 viewPosFromDepth(float depth, vec2 screenPos, mat4 invProj)
{
  vec4 clipPos;
  clipPos.xy = screenPos * 2.0 - 1.0;
  clipPos.z = depth;
  clipPos.w = 1.0;

  vec4 homoCoord = invProj * clipPos;
  return homoCoord.xyz / homoCoord.w;
}

layout(set = 0, binding = 0) uniform sampler2D normals;
layout(set = 0, binding = 2) uniform sampler2D albedos;
layout(set = 0, binding = 4) uniform sampler2D depth;
layout(set = 0, binding = 5) uniform sampler2D uvs;
layout(set = 0, binding = 6) uniform sampler2D ssao;
layout(set = 0, binding = 8) uniform sampler2D diffuseAtlas;

layout(set = 1, binding = 2) uniform sampler2D probeSamplers[4];


struct Material {
  vec4 colour;
  vec2 atlasMin;
  vec2 atlasMax;
};

layout(std140, set = 3, binding = 0) readonly buffer Materials {
   Material materials[];
};


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
  mat4 invProj;	
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

vec2 repeatUV(vec2 uv, vec2 width)
{
  return fract(uv) * width;
}

void main() {
  


  
  vec3 col = texture(albedos, fragTexCoord).rgb;

  vec3 normal = texture(normals, fragTexCoord).xyz;
  //outColour = vec4(normal, 1.0);
  //return;

  float depth = texture(depth, fragTexCoord).r;

  vec3 worldPos = worldPosFromDepth(depth, fragTexCoord, _MainCamera.invViewProj);
  
  //vec3 normalDirection = normalize(varyingNormalDirection);
  vec3 fiberDirection = normalize(vec3(1.0,0.0,1.0));
  vec3 tangentDirection = normalize(cross(normal, fiberDirection));

  
  vec4 cameraPosV = _MainCamera.invViewProj * vec4(0.0,0.0,0.0,1.0);
  vec3 cameraPos = cameraPosV.xyz / cameraPosV.w;
  
  vec3 viewDirection = normalize(cameraPos - worldPos);
  vec3 lightDirection = normalize(vec3(1.0,1.0,0.0));
  float attenuation = 1.0;

  
  vec3 halfwayVector = 
    normalize(lightDirection + viewDirection);
  vec3 binormal = 
    cross(normal, tangentDirection);
  float dotLN = dot(lightDirection, normal); 
  // compute this dot product only once
  vec3 matDiffuseColor = col;

  float ssaoVal = 1.0;//texture(ssao, fragTexCoord).r;
  vec3 ambientLighting = vec3(0.3) * col;
  vec3 lightColor0 = vec3(1.0,1.0,1.0);

  /*

    WARD ANISO
    
   */
    
  vec3 diffuseReflection = attenuation * lightColor0 
    * vec3(matDiffuseColor) * max(0.0, dotLN);
            
  vec3 specularReflection = vec3(0.0);

  if (dotLN < 0.0) // light source on the wrong side?
    {
      specularReflection = vec3(0.0, 0.0, 0.0); 
      // no specular reflection
    }
  else // light source on the right side
    {
      float dotHN = dot(halfwayVector, normal);
      float dotVN = dot(viewDirection, normal);
      //float _AlphaX = cascadeInfo.bilateralBlend;
      //float _AlphaY = cascadeInfo.end;
      float _AlphaX = 0.1;
      float _AlphaY = 0.1;
      
      float dotHTAlphaX = 
	dot(halfwayVector, tangentDirection) / _AlphaX;
      float dotHBAlphaY = dot(halfwayVector, 
			      binormal) / _AlphaY;

      vec3 matSpecularColor = vec3(0.9,0.9,0.9);
      
      specularReflection = attenuation * vec3(matSpecularColor) 
	* sqrt(max(0.0, dotLN / dotVN)) 
	* exp(-2.0 * (dotHTAlphaX * dotHTAlphaX 
		      + dotHBAlphaY * dotHBAlphaY) / (1.0 + dotHN));
    }


  outColour = vec4( ssaoVal * (ambientLighting 
			       + diffuseReflection) + specularReflection , 1.0);

  if (normal == vec3(0.0,0.0,0.0)) //debug mat
    {
      outColour = vec4(1.0,0.0,1.0,1.0);
    }

  
  return;

    /*
  vec4 radiance = vec4(0.0);
  
  int cascade = cascadeInfo.cascade;
  //Debug show outside of range
  
  int dirCount = int(pow(4, cascade));
  int dirTilingCount = int(pow(2, cascade));

  //  vec2 texCoord = quadrantLocalCoord + quadrantOffset;
  radiance = texture(probeSamplers[0], fragTexCoord);
  //radiance += val / dirCount;

  radiance /= radiance.a;
  //  radiance = 4.0; //Hlaf of rays point inwards?
  //radiance.a = 1.0;
  //  outColour = radiance * albedo;

  ////outColour = toSRGB(radiance);
  outColour = radiance;
  return;
  vec3 ambientLighting = radiance.rgb;
  */
  
  //return;
}
