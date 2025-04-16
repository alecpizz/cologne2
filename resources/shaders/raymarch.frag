#version 460 core

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D sNormal;
layout (binding = 1) uniform sampler2D gWorld;
layout (binding = 2) uniform sampler2D gDepth;
layout (location = 0) out vec4 sGI;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    vec4 rayDir = vec4(texture(sNormal, TexCoords).xyz, 1.0);
    rayDir = rayDir * view * projection;
    vec4 rayStart = vec4(texture(gWorld, TexCoords).xyz, 1.0);
    rayStart = rayStart * view * projection;
}