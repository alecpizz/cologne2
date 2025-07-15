#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

uniform mat4 lightSpaceMatrix;

layout (binding = 4, std430) restrict readonly buffer model_mat_buffer
{
    mat4[] model_matrices;
};

void main()
{
    mat4 model = model_matrices[gl_DrawID];
    vec4 vertex_position = vec4(position, 1.0f);
    vertex_position += vec4(normal, 0.0) * 0.0005f;
    gl_Position = lightSpaceMatrix * model * vec4(vertex_position.xyz, 1.0);
}