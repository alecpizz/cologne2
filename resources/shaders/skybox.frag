#version 460 core
out vec4 FragColor;
in vec3 LocalPosition;

layout (binding = 0) uniform samplerCube environment_map;

void main()
{
    vec3 envColor = textureLod(environment_map, LocalPosition, 1.0).rgb;
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0 / 2.2));
    FragColor = vec4(envColor, 1.0);
}