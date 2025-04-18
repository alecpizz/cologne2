#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 voxel_size;

out vec4 g_world_pos;
out vec2 g_tex_coords;

void main()
{
    g_world_pos = model * vec4(position, 1.0f);
    g_tex_coords = uv;
    gl_Position = vec4(g_world_pos.xyz * voxel_size, 1.0f);
}