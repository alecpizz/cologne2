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

out flat int Probe_index_x;
out flat int Probe_index_y;
out flat int Probe_index_z;

out vec3 Color;

void main()
{
    const float scale = 0.03;
    const int indexZ = gl_InstanceID % depth;
    const int indexY = (gl_InstanceID / depth) % height;
    const int indexX = gl_InstanceID / (height * depth);

    float x = indexX * spacing;
    float y = indexY * spacing;
    float z = indexZ * spacing;
    x += origin.x;
    y += origin.y;
    z += origin.z;

    const mat4 model = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
              x,   y,   z, 1.0
    );

    Probe_index_x = indexX;
    Probe_index_y = indexY;
    Probe_index_z = indexZ;

    Color = vec3(1.0, 0.0, 0.0);
    gl_Position = projection * view * model * vec4(position * scale, 1.0);
}