#version 460 core
layout (location = 0) in vec3 position;
out vec2 TexCoord;

void main()
{
    TexCoord = (position.xy + vec2(1.0f)) * 0.5f;
    gl_Position = vec4(position, 1.0);
}