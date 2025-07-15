#version 460 core
#extension GL_ARB_shader_viewport_layer_array: enable
#extension GL_AMD_vertex_shader_layer: enable
#extension GL_NV_viewport_array2: enable
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;


layout (binding = 4, std430) restrict readonly buffer model_mat_buffer
{
    mat4[] model_matrices;
};


uniform mat4 light_space_matrices[6];
out vec4 FragPos;

void main()
{
    mat4 model = model_matrices[gl_DrawID];
    vec4 vertex_position = vec4(position, 1.0f);
    vertex_position += vec4(normal, 1.0) * 0.0005f;
    gl_Layer = gl_InstanceID - gl_BaseInstance;
    FragPos = light_space_matrices[gl_Layer] * model * vertex_position;
    gl_Position = FragPos;
}