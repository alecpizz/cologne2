#version 460 core

in vec4 FragPos;

uniform vec3 light_position;
uniform float far_plane = 20.0f;

void main()
{
//    float dist = length(FragPos.xyz - light_position);
//    dist = dist / far_plane;
//    gl_FragDepth = dist;
}