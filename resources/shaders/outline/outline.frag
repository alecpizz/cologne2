#version 460

layout (location = 0) out vec4 FragColor;
layout (r8, binding = 0) uniform image2D outline_mask;

uniform int offset_count;
uniform ivec2 offsets[256];

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    vec4 camera_position;
};

flat in int offset_idx;

void main()
{
    ivec2 pixel_coord = ivec2(gl_FragCoord.xy) + offsets[offset_idx];

    ivec2 image_res = imageSize(outline_mask);

    pixel_coord.x = clamp(pixel_coord.x, 0, image_res.x - 1);
    pixel_coord.y = clamp(pixel_coord.y, 0, image_res.y - 1);

    float mask = imageLoad(outline_mask, pixel_coord).r;
    FragColor = vec4(mask, 0.0, 0.0, 0.0);
}