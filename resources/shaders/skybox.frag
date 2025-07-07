#version 460 core
layout (location = 0) out vec4 Position;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 FragColor;
layout (location = 4) out vec3 gEmission;
in vec3 LocalPosition;
in vec4 WorldPosition;

layout (binding = 0) uniform samplerCube environment_map;

void main()
{
    vec3 envColor = textureLod(environment_map, LocalPosition, 1.0).rgb;
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0 / 2.2));
    FragColor = vec4(envColor, 1.0);
    Position = WorldPosition;
    gEmission = vec3(0.0f);
    gNormal = vec4(0.0f);
}