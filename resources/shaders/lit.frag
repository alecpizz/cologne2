#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = vec4(0.5, 0.5, 0.0, 1.0);
}