#version 460 core

layout (location = 0) in vec3 position;

layout (binding = 1, std430) readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    vec4 camera_position;
};


uniform mat4 model;
out vec3 WorldPosition;

void main()
{
    WorldPosition = vec3(model * vec4(position, 1.0));
    gl_Position = projection_view * vec4(WorldPosition, 1.0);
}