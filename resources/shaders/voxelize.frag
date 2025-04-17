#version 460 core

layout (binding = 0) uniform sampler2D texture_albedo;
layout (binding = 1) uniform sampler2D texture_ao;
layout (binding = 2) uniform sampler2D texture_metallic;
layout (binding = 3) uniform sampler2D texture_roughness;
layout (binding = 4) uniform sampler2D texture_normal;
layout (binding = 5) uniform sampler2D texture_emission;

uniform int voxel_size;
layout(RGBA8, binding = 6) uniform image3D texture_voxel;

in vec2 TexCoord;
in flat int Axis;

void main()
{
    vec4 color = texture(texture_albedo, TexCoord);

    ivec3 camera = ivec3(gl_FragCoord.x, gl_FragCoord.y, voxel_size * gl_FragCoord.z);
    ivec3 voxelPos;

    if(Axis == 1)
    {
        voxelPos.x = voxel_size - 1 - camera.z;
        voxelPos.z = voxel_size - 1 - camera.x;
        voxelPos.y = camera.y;
    }
    else if(Axis == 2)
    {
        voxelPos.z = voxel_size - 1 - camera.y;
        voxelPos.y = voxel_size - 1 - camera.z;
        voxelPos.x = camera.x;
    }
    else
    {
        voxelPos = camera;
        voxelPos.z = voxel_size - 1 - camera.z;
    }

    imageStore(texture_voxel, voxelPos, vec4(color.rgb, 1.0f));
}