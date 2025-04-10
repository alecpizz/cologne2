#version 460 core

layout (location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler3D probe_lighting;
in vec3 Color;

in flat int ProbeIndex;


void main()
{
    FragColor.rgb = Color;
    FragColor.a = 1.0f;
}