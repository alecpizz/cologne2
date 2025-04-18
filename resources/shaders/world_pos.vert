#version 460 core

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 WorldPosition;

void main()
{
    WorldPosition = vec3(model * vec4(position, 1.0));
    gl_Position = projection * view * vec4(WorldPosition, 1.0);
}