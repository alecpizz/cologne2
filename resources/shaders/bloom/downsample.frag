#version 460 core

layout (binding = 0) uniform sampler2D src_texture;
uniform vec2 src_resolution;

in vec2 TexCoords;
layout (location = 0) out vec3 downsample;

void main()
{
    vec2 src_texel_size = src_resolution;
    float x = src_texel_size.x;
    float y = src_texel_size.y;

    vec3 a = texture(src_texture, vec2(TexCoords.x - 2*x, TexCoords.y + 2*y)).rgb;
    vec3 b = texture(src_texture, vec2(TexCoords.x,       TexCoords.y + 2*y)).rgb;
    vec3 c = texture(src_texture, vec2(TexCoords.x + 2*x, TexCoords.y + 2*y)).rgb;

    vec3 d = texture(src_texture, vec2(TexCoords.x - 2*x, TexCoords.y)).rgb;
    vec3 e = texture(src_texture, vec2(TexCoords.x,       TexCoords.y)).rgb;
    vec3 f = texture(src_texture, vec2(TexCoords.x + 2*x, TexCoords.y)).rgb;

    vec3 g = texture(src_texture, vec2(TexCoords.x - 2*x, TexCoords.y - 2*y)).rgb;
    vec3 h = texture(src_texture, vec2(TexCoords.x,       TexCoords.y - 2*y)).rgb;
    vec3 i = texture(src_texture, vec2(TexCoords.x + 2*x, TexCoords.y - 2*y)).rgb;

    vec3 j = texture(src_texture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 k = texture(src_texture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 l = texture(src_texture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 m = texture(src_texture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    downsample = e * 0.125;
    downsample += (a + c + g + i) * 0.03125f;
    downsample += (b + d + f + h) * 0.0625f;
    downsample += (j + k + l + m) * 0.125f;
}