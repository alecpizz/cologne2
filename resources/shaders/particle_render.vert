#version 460 core

layout (std430, binding = 4) buffer position
{
    vec4 positions[];
};


layout (binding = 1, std430) restrict readonly buffer viewportdata
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

    gl_Position = projection_view * particle_pos;

    gl_PointSize = 3.5;

    FragPos = particle_pos.xyz;
}