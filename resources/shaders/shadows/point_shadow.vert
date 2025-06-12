#version 460 core
#extension GL_ARB_shader_viewport_layer_array: enable
#extension GL_AMD_vertex_shader_layer: enable
#extension GL_NV_viewport_array2: enable
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in ivec4 boneID;
layout (location = 5) in vec4 weight;

uniform mat4 model;
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 bone_matrices[MAX_BONES];
uniform mat4 light_space_matrices[6];
uniform bool is_skinned = false;
out vec4 FragPos;
void main()
{
    vec4 vertex_position = vec4(0.0f);
    if (is_skinned)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneID[i] < 0)
            continue;
            if (boneID[i] >= MAX_BONES)
            {
                vertex_position = vec4(position, 1.0f);
                break;
            }
            vec4 localPosition = bone_matrices[boneID[i]] * vec4(position, 1.0f);
            vertex_position += localPosition * max(weight[i], 0.0f);
        }
    }
    else
    {
        vertex_position = vec4(position, 1.0f);
    }
    gl_Layer = gl_InstanceID;
    FragPos = light_space_matrices[gl_Layer] * model * vertex_position;
    gl_Position = FragPos;
}