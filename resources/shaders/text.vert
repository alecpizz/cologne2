#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;

out vec4 Color;
out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * vec4(position, 1.0);
    Color = color;
    TexCoord = uv;
}