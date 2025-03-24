#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Position;
out vec4 FragPos;

void main()
{
    Normal = mat3(transpose(inverse(model))) * normal;
    vec4 worldPos = model * vec4(position, 1.0);
    gl_Position = lightSpaceMatrix * model * vec4(position, 1.0);
    TexCoords = uv;
    Position = worldPos.xyz;
    FragPos = worldPos;
}
