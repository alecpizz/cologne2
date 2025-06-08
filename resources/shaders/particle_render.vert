#version 460 core

layout (std430, binding = 0) buffer position
{
    vec4 positions[];
};

layout (std430, binding = 1) buffer velocity
{
    vec4 velocities[];
};

layout (binding = 1, std430) readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    vec4 camera_position;
};


out vec3 FragPos;

void main()
{
    vec4 particle_pos = positions[gl_InstanceID];

    gl_Position = projection * view * particle_pos;

    gl_PointSize = 3.5;

    FragPos = particle_pos.xyz;
}