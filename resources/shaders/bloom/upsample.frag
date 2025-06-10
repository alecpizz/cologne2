#version 460 core

layout (binding = 0) uniform sampler2D src_texture;
uniform float filter_radius = 0.005f;
uniform float src_aspect_ratio;

in vec2 TexCoords;
layout (location = 0) out vec3 upsample;

void main()
{
    float x = filter_radius;
    float y = filter_radius * src_aspect_ratio;

    vec3 a = texture(src_texture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 b = texture(src_texture, vec2(TexCoords.x,     TexCoords.y + y)).rgb;
    vec3 c = texture(src_texture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;

    vec3 d = texture(src_texture, vec2(TexCoords.x - x, TexCoords.y)).rgb;
    vec3 e = texture(src_texture, vec2(TexCoords.x,     TexCoords.y)).rgb;
    vec3 f = texture(src_texture, vec2(TexCoords.x + x, TexCoords.y)).rgb;

    vec3 g = texture(src_texture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 h = texture(src_texture, vec2(TexCoords.x,     TexCoords.y - y)).rgb;
    vec3 i = texture(src_texture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    upsample = e * 4.0f;
    upsample += (b + d + f + h);
    upsample += (a + c + g + i);
    upsample *= 1.0f / 16.0f;
}