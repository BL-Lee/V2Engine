#version 450

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler3D probeSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {

    outColor = texture(texSampler, fragTexCoord);
}