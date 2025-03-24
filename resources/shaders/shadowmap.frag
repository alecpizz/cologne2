#version 460 core
in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

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
    normal = Normal;
    worldPos = Position;

    //flux calc here
    vec3 lightDir = normalize(-light.direction);
    vec3 norm = normalize(Normal);
    float diff = max(0.0, dot(norm, lightDir));
    flux = diff * texture2D(albedo, TexCoords).rgb * light.color;
}