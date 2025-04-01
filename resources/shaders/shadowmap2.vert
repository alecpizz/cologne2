#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
uniform mat4 model;

void main()
{
    float grow = 1.1;
    vec4 P = vec4(position, 1.0);
    P.xyz += normal * grow;
    gl_Position = model * P;
}