#version 460 core

in vec3 WorldPosition;

out vec4 Color;

void main()
{
    Color.rgb = WorldPosition;
}