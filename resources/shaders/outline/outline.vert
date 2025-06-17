#version 460 core

layout (location = 0) in vec3 position;

flat out int offset_idx;

void main()
{
    offset_idx = gl_InstanceID;

    gl_Position = vec4(position * 2.0f, 1.0f);
}