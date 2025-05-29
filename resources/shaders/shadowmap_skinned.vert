#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in ivec4 boneID;
layout (location = 5) in vec4 weight;

uniform mat4 model;
out vec2 v_TexCoords;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 bone_matrices[MAX_BONES];


void main()
{
    float grow = 1.1;
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneID[i] == -1)
            continue;
        if(boneID[i] >=MAX_BONES)
        {
            totalPosition = vec4(position,1.0f);
            break;
        }
        vec4 localPosition = bone_matrices[boneID[i]] * vec4(position,1.0f);
        totalPosition += localPosition * weight[i];
    }


    vec4 P = totalPosition;
//    P.xyz += normal * grow;
    gl_Position = model * P;

    v_TexCoords = uv;
}