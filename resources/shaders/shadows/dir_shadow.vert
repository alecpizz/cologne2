#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in ivec4 boneID;
layout (location = 5) in vec4 weight;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform bool is_skinned = false;
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 bone_matrices[MAX_BONES];

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
    vertex_position += vec4(normal, 1.0) * 0.0005f;
    gl_Position = lightSpaceMatrix * model * vec4(vertex_position.xyz, 1.0);
}