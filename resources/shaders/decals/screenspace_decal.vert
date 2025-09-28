#version 460 core

layout (location = 0) in vec3 position;

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    mat4 projection_view_inverse;
    vec4 camera_position;
};

uniform mat4 model;


void main()
{
    gl_Position = projection_view * model * vec4(position, 1.0);
}