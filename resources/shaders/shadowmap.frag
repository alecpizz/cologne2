#version 460 core
in vec4 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;
uniform vec3 lightPos;
uniform float far_plane;
layout (binding = 0) uniform sampler2D albedo;
layout (location = 0) out vec3 normal;
layout (location = 1) out vec3 worldPos;
layout (location = 2) out vec3 flux;

void main()
{
    if(texture(albedo, TexCoords).a < 0.5)
    {
        discard;
    }
    gl_FragDepth = gl_FragCoord.z;
    // get distance between fragment and light source
    normal = normalize(Normal);
    worldPos = FragPos.xyz;

    //flux calc here
    vec3 lightPos = vec3(0.0f, 10.0f, 0.0f);
    vec3 lightCol = vec3(300.0f, 300.0f, 300.0f);
    float distance = length(lightPos - Position);
    vec3 lightDir = normalize(lightPos - Position);
    float attenuation = 1.0 / (distance * distance);
    float diff = max(0.0, dot(normalize(normal.xyz), lightDir));
    vec3 radiance = lightCol * attenuation;
    flux = texture2D(albedo, TexCoords).xyz * radiance;
}