#version 460 core

layout (binding = 0) uniform sampler2D texture_albedo;
layout (binding = 1) uniform sampler2D texture_ao;
layout (binding = 2) uniform sampler2D texture_metallic;
layout (binding = 3) uniform sampler2D texture_roughness;
layout (binding = 4) uniform sampler2D texture_normal;
layout (binding = 5) uniform sampler2D texture_emission;

layout (RGBA8, binding = 6) uniform image3D texture_voxel;
uniform vec3 voxel_size;
//in vec3 f_normal;
in vec2 f_tex_coords;
in vec3 f_voxel_pos;
in vec4 f_shadow_coords;

bool is_inside_clipspace(const vec3 p)
{
    return abs(p.x) < 1 && abs(p.y) < 1 && abs(p.z) < 1;
}

vec3 from_clipspace_to_texcoords(vec3 p)
{
    return 0.5f * p + vec3(0.5f);
}

void main()
{
    if (!is_inside_clipspace(f_voxel_pos))
    {
        return;
    }
    vec4 color = texture(texture_albedo, f_tex_coords);
    color.a = 1.0f;
    vec3 voxelgrid_tex_pos = from_clipspace_to_texcoords(f_voxel_pos);
    ivec3 voxelgrid_resolution = imageSize(texture_voxel);
    imageStore(texture_voxel, ivec3(voxelgrid_resolution * voxelgrid_tex_pos), color);
}