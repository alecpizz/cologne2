#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform int depth;
uniform int width;
uniform int height;
uniform float spacing;
uniform vec3 origin;

out flat int ProbeIndex;
out vec3 ProbeWorldPos;
out vec3 FragPos;

layout (binding = 1, std430) readonly buffer probe_positions
{
    vec4 positions[];
};


out vec3 Color;

void main()
{
    ProbeIndex = gl_InstanceID;
    const float scale = 0.03;

    vec3 pos = positions[ProbeIndex].xyz;
    ProbeWorldPos = pos;

    const mat4 model = mat4(
    0.03, 0.0, 0.0, 0.00,
    0.0, 0.03, 0.0, 0.00,
    0.0, 0.0, 0.03, 0.00,
    pos.x, pos.y, pos.z, 1.0
    );

    Color = vec3(1.0, 0.0, 0.0);
    FragPos = vec3(model * vec4(position, 1.0));
    gl_Position = projection * view * model * vec4(position, 1.0);
}