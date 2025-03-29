#version 460 core

in vec4 Color;
in vec2 TexCoord;

layout (binding = 0) uniform sampler2D textureAtlas;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(textureAtlas, TexCoord).r) * Color;
}