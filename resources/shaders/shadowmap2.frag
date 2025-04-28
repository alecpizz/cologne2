#version 460 core

in vec2 g_TexCoords;

layout (binding = 0) uniform sampler2D albedo;

void main()
{
    if(texture(albedo, g_TexCoords).a < 0.5)
    {
        discard;
    }
    gl_FragDepth = gl_FragCoord.z;
}