#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

layout (binding = 0) uniform sampler2D fbo;

void main()
{
    float depth = texture2D(fbo, TexCoords).r;
    FragColor = vec4(vec3(depth), 1.0);
}