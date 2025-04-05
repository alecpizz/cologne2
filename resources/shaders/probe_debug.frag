#version 460 core

layout (location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler3D probe_lighting;
in vec3 Color;

in flat int Probe_index_x;
in flat int Probe_index_y;
in flat int Probe_index_z;


void main()
{
    vec3 col = texture(probe_lighting, ivec3(Probe_index_x, Probe_index_y, Probe_index_z)).rgb;
    FragColor.rgb = col;
    FragColor.a = 1.0f;
}