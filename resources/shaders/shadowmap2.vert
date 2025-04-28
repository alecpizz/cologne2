#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 model;
out vec2 v_TexCoords;


void main()
{
    float grow = 1.1;
    vec4 P = vec4(position, 1.0);
    P.xyz += normal * grow;
    gl_Position = model * P;

    v_TexCoords = uv;
}