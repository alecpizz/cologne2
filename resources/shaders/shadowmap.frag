#version 460 core
in vec4 FragPos;
in vec2 TexCoords;
uniform vec3 lightPos;
uniform float far_plane;
layout (binding = 0) uniform sampler2D albedo;

void main()
{
    if(texture(albedo, TexCoords).a < 0.5)
    {
        discard;
    }
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;

    // write this as modified depth
    gl_FragDepth = lightDistance;
}