#version 460 core

in vec2 g_TexCoords;
in vec3 g_Normal;
in vec4 g_Position;

layout (binding = 0) uniform sampler2D albedo;
layout (location = 0) out vec3 normal;
layout (location = 1) out vec3 worldPos;
layout (location = 2) out vec3 flux;

struct Light
{
    vec3 position;
    vec3 direction;
    vec3 color;
    float radius;
    float strength;
    int type;
};
uniform Light light;


void main()
{
    if(texture(albedo, g_TexCoords).a < 0.5)
    {
        discard;
    }
    gl_FragDepth = gl_FragCoord.z;

    normal = g_Normal;
    worldPos = g_Position.xyz;

    vec3 lightDir = normalize(-light.direction);
    vec3 norm = normalize(g_Normal);
    float diff = max(0.0, dot(norm, lightDir));
    flux = diff * texture2D(albedo, g_TexCoords).rgb * light.color;// vec3(1.0f, 0.0f, 0.0f);//diff * texture2D(albedo, g_TexCoords).rgb * light.color * 200.0;
}