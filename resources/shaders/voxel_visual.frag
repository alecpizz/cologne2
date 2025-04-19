#version 460 core

layout (binding = 0) uniform sampler3D texture_voxel;
layout (binding = 1) uniform sampler2D texture_cube_back;
layout (binding = 2) uniform sampler2D texture_cube_front;

uniform vec3 camera_position;


in vec2 TexCoord;

out vec4 Color;

bool is_inside_voxelgrid(const vec3 p) {
    return abs(p.x) < 1.1f && abs(p.y) < 1.1f && abs(p.z) < 1.1f;
}

void main()
{
    vec4 accumulated_color = vec4(0, 0, 0, 0);
    vec3 ray_origin = is_inside_voxelgrid(camera_position) ?
    camera_position : texture(texture_cube_front, TexCoord).xyz;
    vec3 ray_end = texture(texture_cube_back, TexCoord).xyz;
    vec3 ray_direction = normalize(ray_end - ray_origin);

    const float ray_step_size = 0.003f;
    int total_samples = int(length(ray_end - ray_origin) / ray_step_size);

    for (int i = 0; i < total_samples; i++)
    {
        vec3 sample_location = (ray_origin + ray_direction * ray_step_size * i);
        vec4 texSample = textureLod(texture_voxel, (sample_location + vec3(1.0f)) * 0.5f, 0);

        if (texSample.a > 0)
        {
            texSample.rgb /= texSample.a;
            accumulated_color.rgb = accumulated_color.rgb + (1.0f - accumulated_color.a) * texSample.a * texSample.rgb;
            accumulated_color.a = accumulated_color.a + (1.0f - accumulated_color.a) * texSample.a;
        }

        if (accumulated_color.a > 0.95)
        {
            break;
        }// early exit
    }

    accumulated_color.rgb = pow(accumulated_color.rgb, vec3(1.0f / 2.2f));
    Color = accumulated_color;
}