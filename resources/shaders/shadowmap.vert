#version 460 core
layout (location = 0) in vec3 position;

uniform mat4 light_projection;
uniform mat4 model;

void main()
{
    gl_Position = light_projection * model * vec4(position, 1.0);
}
