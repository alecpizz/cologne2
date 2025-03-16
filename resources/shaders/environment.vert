#version 460 core
layout (location = 0) in vec3 position;
out vec3 LocalPosition;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    LocalPosition = position;
    gl_Position = projection * view * vec4(LocalPosition, 1.0);
}