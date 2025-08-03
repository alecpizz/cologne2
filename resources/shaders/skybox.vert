#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    mat4 projection_view_inverse;
    vec4 camera_position;
};

uniform mat4 projection_override;

out vec3 LocalPosition;
out vec4 WorldPosition;

void main()
{
    LocalPosition = position;
    mat4 rotView = mat4(mat3(view));
    vec4 clipPos = projection_override * rotView * vec4(position, 1.0);
    gl_Position = clipPos.xyww;
    WorldPosition = clipPos;
}