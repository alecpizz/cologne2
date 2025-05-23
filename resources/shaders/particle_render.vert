#version 460 core

layout (std430, binding = 0) buffer position
{
    vec4 positions[];
};

layout (std430, binding = 1) buffer velocity
{
    vec4 velocities[];
};

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;

void main()
{
    vec4 particle_pos = positions[gl_InstanceID];

    gl_Position = projection * view * particle_pos;

    gl_PointSize = 5.0;

    FragPos = particle_pos.xyz;
}