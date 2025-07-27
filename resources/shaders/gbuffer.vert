#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TBN;
out flat uint EntityID;
out flat uint DrawID;

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    mat4 projection_view_inverse;
    vec4 camera_position;
};

layout (binding = 4, std430) restrict readonly buffer model_mat_buffer
{
    mat4[] model_matrices;
};

void main()
{
    mat4 model = model_matrices[gl_DrawID];
    DrawID = gl_DrawID;
    vec4 worldPos = model * vec4(position, 1.0);
    FragPos = worldPos.xyz;
    TexCoords = uv;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    Normal = normalize(normalMatrix * normal);
    EntityID = gl_BaseInstance;
    gl_Position = projection_view * worldPos;
}