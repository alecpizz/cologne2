#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 model;
out vec2 UV;
out vec3 Norm;
out vec3 Pos;

void main()
{
    vec4 worldPos = model * vec4(position, 1.0);
    gl_Position = worldPos;
    UV = uv;
    Norm = normal;
    Pos = worldPos.xyz;
}
