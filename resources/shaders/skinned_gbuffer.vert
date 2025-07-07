#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in ivec4 boneID;
layout (location = 5) in vec4 weight;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 bone_matrices[MAX_BONES];

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    mat4 projection_view_inverse;
    vec4 camera_position;
};


out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TBN;
out flat uint EntityID;
out flat uint DrawID;
uniform mat4 model;
uniform uint entity_id;

void main()
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


    vec4 worldPos = model * totalPosition;
    FragPos = worldPos.xyz;
    TexCoords = uv;

    mat4 bone_transform = bone_matrices[boneID[0]] * weight[0];
    bone_transform += bone_matrices[boneID[1]] * weight[1];
    bone_transform += bone_matrices[boneID[2]] * weight[2];
    bone_transform += bone_matrices[boneID[3]] * weight[3];

    mat3 normalMatrix = transpose(inverse(mat3(bone_transform)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    Normal = normalMatrix * normal;
    EntityID = entity_id;
    DrawID = 0; //temp
    gl_Position = projection_view * worldPos;
}