#version 460 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in mat3 TBN;

layout (binding = 0) uniform sampler2D texture_albedo;
layout (binding = 1) uniform sampler2D texture_normal;

void main()
{
    gPosition = vec4(FragPos, 1.0);
    vec3 N = texture2D(texture_normal, TexCoords).rgb;
    N = N * 2.0 - 1.0;
    N = normalize(TBN * N);
    gNormal = vec4(N, 1.0);
    gl_FragDepth = gl_FragCoord.z;
    vec4 albedo = texture(texture_albedo, TexCoords).rgba;
    if(albedo.a < 0.5)
    {
        discard;
    }
    gAlbedo.rgba = albedo;
}