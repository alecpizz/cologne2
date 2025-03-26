#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 view;
uniform mat4 projection;
out vec4 Color;

void main()
{
    Color = vec4(color, 1.0);
    gl_Position = projection * view * vec4(position, 1.0);
}