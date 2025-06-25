#version 460 core

layout (binding = 0) uniform sampler3D texture_voxel;
layout (binding = 1) uniform sampler2D texture_cube_back;
layout (binding = 2) uniform sampler2D texture_cube_front;
layout (binding = 3) uniform usampler3D texture_voxel_normal;

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    vec4 camera_position;
};


in vec2 TexCoord;

out vec4 Color;

bool is_inside_voxelgrid(const vec3 p) {
    return abs(p.x) < 1.1f && abs(p.y) < 1.1f && abs(p.z) < 1.1f;
}


vec3 decode_normal(uint data)
{
    uvec2 iv = uvec2( data, data>>16u ) & 65535u; vec2 v = vec2(iv)/32767.5 - 1.0;
    vec3 nor = vec3(v, 1.0 - abs(v.x) - abs(v.y)); // Rune Stubbe's version,
    float t = max(-nor.z,0.0);                     // much faster than original
    nor.x += (nor.x>0.0)?-t:t;                     // implementation of this
    nor.y += (nor.y>0.0)?-t:t;                     // technique
    return normalize( nor );
}

void main()
{
    vec4 accumulated_color = vec4(0, 0, 0, 0);
    vec4 accumulated_normal = vec4(0);
    vec3 ray_origin = is_inside_voxelgrid(camera_position.xyz) ?
    camera_position.xyz : texture(texture_cube_front, TexCoord).xyz;
    vec3 ray_end = texture(texture_cube_back, TexCoord).xyz;
    vec3 ray_direction = normalize(ray_end - ray_origin);

    const float ray_step_size = 0.003f;
    int total_samples = int(length(ray_end - ray_origin) / ray_step_size);

    for (int i = 0; i < total_samples; i++)
    {
        vec3 sample_location = (ray_origin + ray_direction * ray_step_size * i);
        vec4 texSample = textureLod(texture_voxel, (sample_location + vec3(1.0f)) * 0.5f, 0);
        uint norm_sample = textureLod(texture_voxel_normal, vec3((sample_location + vec3(1.0f)) * 0.5f), 0).r;
        vec3 decode_normal = decode_normal(norm_sample);
        if (texSample.a > 0)
        {
            texSample.rgb /= texSample.a;
            bool view_normals = true;
            if(view_normals)
            {   
                accumulated_normal.rgb = accumulated_normal.rgb + (1.0f - accumulated_normal.a) * texSample.a * decode_normal.rgb;
            }
            accumulated_color.rgb = accumulated_color.rgb + (1.0f - accumulated_color.a) * texSample.a * texSample.rgb;
            accumulated_color.a = accumulated_color.a + (1.0f - accumulated_color.a) * texSample.a;
        }

        if (accumulated_color.a > 0.95)
        {
            break;
        }// early exit
    }

//    accumulated_color.rgb = accumulated_color.rgb / (accumulated_color.rgb + vec3(1.0));
//    accumulated_color.rgb = pow(accumulated_color.rgb, vec3(1.0f / 2.2f));
    Color = accumulated_color;
}