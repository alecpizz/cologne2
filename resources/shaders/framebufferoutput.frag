#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

layout (binding = 0) uniform sampler2D fbo;

void main()
{
    vec3 color =  texture(fbo, TexCoords).rgb;
    FragColor = vec4(color.r, color.r, color.r, 1.0);
}