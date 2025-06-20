#version 460 core
layout (location = 0) in vec3 position;
layout (location = 4) in ivec4 boneID;
layout (location = 5) in vec4 weight;

uniform mat4 model;
uniform bool is_skinned = false;
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 bone_matrices[MAX_BONES];

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    vec4 camera_position;
};

void main()
{
    if (is_skinned)
    {
        vec4 totalPosition = vec4(0.0f);
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(boneID[i] < 0)
            continue;
            if(boneID[i] >= MAX_BONES)
            {
                totalPosition = vec4(position,1.0f);
                break;
            }
            vec4 localPosition = bone_matrices[boneID[i]] * vec4(position,1.0f);
            totalPosition += localPosition * max(weight[i], 0.0f);
        }
        vec4 world_pos = model * totalPosition;
        gl_Position = projection_view * world_pos;
    }
    else
    {
        vec4 world_pos = model * vec4(position, 1.0);
        gl_Position = projection_view * world_pos;
    }
}