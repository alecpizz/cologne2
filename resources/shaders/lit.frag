#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 WorldPos;
in mat3 TBN;

struct Light
{
    vec3 position;
    vec3 direction;
    vec3 color;
    float radius;
    float strength;
    int type;
};

#define MAX_LIGHTS 8
#define PI 3.1415926535897932384626433832795
#define DIRECTIONAL 0
#define POINT 1

uniform vec3 camera_pos;
uniform Light lights[MAX_LIGHTS];
uniform int num_lights = 0;

layout (binding = 0) uniform sampler2D texture_albedo;
layout (binding = 1) uniform sampler2D texture_ao;
layout (binding = 2) uniform sampler2D texture_metallic;
layout (binding = 3) uniform sampler2D texture_roughness;
layout (binding = 4) uniform sampler2D texture_normal;
layout (binding = 5) uniform sampler2D texture_emission;

layout (binding = 6) uniform samplerCube irradiance_map;
layout (binding = 7) uniform samplerCube prefilter_map;
layout (binding = 8) uniform sampler2D brdf;

layout (binding = 9) uniform samplerCube shadow_map;
layout (binding = 10) uniform samplerCube normal_map;
layout (binding = 11) uniform samplerCube position_map;
layout (binding = 12) uniform samplerCube flux_map;

uniform float far_plane = 20.0f;
uniform float ao_strength = 0.2;
uniform int has_ao_texture = 0;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
float klemenVisibility(vec3 L, vec3 H);

vec3 sampleOffsetDirections[20] = vec3[]
(
vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

uint xorshift(uint state)
{
    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state << 5);
    return state;
}

float randomFloat(inout uint seed)
{
    seed = xorshift(seed);
    return float(seed) / float(0xFFFFFFFFu);
}

vec3 get_random_vector(int index)
{
    uint seed = uint(index);
    float x = randomFloat(seed);
    float y = randomFloat(seed);
    float z = randomFloat(seed);
    return vec3(x * 2.0 - 1.0, y * 2.0 - 1.0, z * 2.0 - 1.0);
}


vec3 indirectLighting(vec3 uvFrag, vec3 n, vec3 x)
{
    vec3 result = vec3(0.0);
    const int samples = 64;
    const float sampleRadius = 0.09;
    for(int i = 0; i < samples; i++)
    {
        vec3 rand = get_random_vector(i);
        vec3 uv = uvFrag * sampleRadius * rand;
        vec3 flux = texture(flux_map, uv).rgb;
        vec3 x_p = texture(position_map, uv).xyz;
        vec3 n_p = texture(normal_map, uv).xyz;

        vec3 r = x - x_p;
        float d2 = dot(r, r);
        vec3 E_p = flux * (max(0.0, dot(n_p, r)) * max(0.0, dot(n, -r)));
        E_p *= rand.x * rand.x / (d2 * d2);
        result += E_p;
    }
    const float intensity = 7.5;
    return result * intensity;
}

void main()
{
    vec4 albedo_texture = texture2D(texture_albedo, TexCoords).rgba;
    if (albedo_texture.a < 0.5)
    {
        discard;
    }
    vec3 albedo = pow(albedo_texture.rgb, vec3(2.2));

    float metallic = texture2D(texture_metallic, TexCoords).b;
    float roughness = texture2D(texture_roughness, TexCoords).g;
    float ao = 0.0;
    if (has_ao_texture == 1)
    {
        ao = texture2D(texture_ao, TexCoords).r;
    }
    else
    {
        ao = ao_strength;
    }

    vec3 N = texture2D(texture_normal, TexCoords).rgb;
    N = N * 2.0 - 1.0;
    N = normalize(TBN * N);

    vec3 V = normalize(camera_pos - WorldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    vec3 lightDirection = vec3(0.0);


    vec3 fragToLight = FragPos - lights[0].position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(camera_pos - FragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;


    for (int i = 0; i < samples; i++)
    {
        vec3 sample_coord = fragToLight + sampleOffsetDirections[i] * diskRadius;

        float closestDepth = texture(shadow_map, sample_coord).r;
        closestDepth *= far_plane;
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

    shadow /= float(samples);
    shadow = 1.0 - shadow;

    vec3 indirect = indirectLighting(fragToLight, N, FragPos);


    for (int i = 0; i < num_lights; i++)
    {
        vec3 L = vec3(0.0);
        if (lights[i].type == DIRECTIONAL)
        {
            L = normalize(-lights[i].direction);
            lightDirection = L;
        }
        else if (lights[i].type == POINT)
        {
            L = normalize(lights[i].position - WorldPos);
        }
        vec3 H = normalize(V + L);

        float distance = length(lights[i].position - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lights[i].color * attenuation;

        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += ((kD * albedo / PI + specular) * radiance * NdotL) * shadow;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(irradiance_map, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_RELFECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilter_map, R, roughness * MAX_RELFECTION_LOD).rgb;
    vec2 brdf = texture(brdf, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);




    vec3 ambient = (kD * (diffuse) + (specular)) * ao;
    vec3 emission = texture2D(texture_emission, TexCoords).rgb;
    vec3 color =   Lo + emission + indirect;


    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));


    FragColor = vec4(color, 1.0);
}


float klemenVisibility(vec3 L, vec3 H)
{
    float LoH = dot(L, H);
    return 0.25 / (LoH * LoH);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 2.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}