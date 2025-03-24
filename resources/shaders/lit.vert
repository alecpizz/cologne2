#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

out vec2 TexCoords;
out vec3 WorldPos;
out mat3 TBN;
out vec3 FragPos;
out vec4 FragPosLightSpace;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    WorldPos = vec3(model * vec4(position, 1.0));
    gl_Position = projection * view * vec4(WorldPos, 1.0);
    TexCoords = uv;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    FragPos = vec3(model * vec4(position, 1.0));
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}