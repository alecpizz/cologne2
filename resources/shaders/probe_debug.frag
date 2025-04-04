#version 460 core

layout (location = 0) out vec4 FragColor;

in vec3 Color;

in flat int Probe_index_x;
in flat int Probe_index_y;
in flat int Probe_index_z;

void main()
{
    FragColor.rgb = Color;
    FragColor.a = 1.0f;
}