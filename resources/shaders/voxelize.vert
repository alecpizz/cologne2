#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

uniform mat4 model;
out vec2 v_TexCoord;

void main()
{
    v_TexCoord = uv;
    gl_Position = model * vec4(position, 1.0f);
}