#version 460 core

layout(binding = 0) uniform sampler3D texture_voxel;

uniform vec3 camera_position;
uniform int state = 0;
in vec3 FragPos;
in vec3 TexCoord;

out vec4 Color;


void main()
{
    vec4 voxelData = texture(texture_voxel, TexCoord);
    if (voxelData.a < 0.01)
    {
        discard;
    }
    Color.rgb = pow(voxelData.rgb, vec3(1.0 / 2.2f));
}