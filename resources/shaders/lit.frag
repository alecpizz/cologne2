#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 WorldPos;
in mat3 TBN;
in vec4 FragPosLightSpace;

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

layout (binding = 9) uniform sampler2D shadow_map;
layout (binding = 10) uniform sampler2D normal_map;
layout (binding = 11) uniform sampler2D position_map;
layout (binding = 12) uniform sampler2D flux_map;

uniform float far_plane = 20.0f;
uniform float ao_strength = 0.2;
uniform int has_ao_texture = 0;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
float klemenVisibility(vec3 L, vec3 H);
float shadowCalculation(vec4 fragPos, vec3 n, vec3 l);

vec3 sampleOffsetDirections[20] = vec3[]
(
vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

float radical_inverse_vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint n)
{
    return vec2(float(i) / float(n), radical_inverse_vdc(i));
}

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
    return vec3(x * 2.0 - 1.0, y * 2.0 - 1.0, z);
}


//vec3 indirectLighting(vec3 uvFrag, vec3 n, vec3 x)
//{
//    vec3 result = vec3(0.0);
//    const int samples = 64;
//    const float sampleRadius = 0.09;
//    for(int i = 0; i < samples; i++)
//    {
//        vec3 rand = get_random_vector(i);
//        vec3 uv = uvFrag * sampleRadius * rand;
//        vec3 flux = texture(flux_map, uv).rgb;
//        vec3 x_p = texture(position_map, uv).xyz;
//        vec3 n_p = texture(normal_map, uv).xyz;
//
//        vec3 r = x - x_p;
//        float d2 = dot(r, r);
//        vec3 E_p = flux * (max(0.0, dot(n_p, r)) * max(0.0, dot(n, -r)));
//        E_p *= rand.x * rand.x / (d2 * d2);
//        result += E_p;
//    }
//    const float intensity = 7.5;
//    return result * intensity;
//}

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
    vec3 lightDirection = normalize(-lights[0].direction);


    vec3 fragToLight = FragPos - lights[0].position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(camera_pos - FragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;



    shadow = 1.0 - shadowCalculation(FragPosLightSpace, N, lightDirection);


//    indirect /= rsmSamples;

    for (int i = 0; i < num_lights; i++)
    {
        vec3 L = vec3(0.0);
        vec3 radiance = vec3(0.0);
        if (lights[i].type == DIRECTIONAL)
        {
            L = normalize(-lights[i].direction);
            radiance = lights[i].color;
        }
        else if (lights[i].type == POINT)
        {
            L = normalize(lights[i].position - WorldPos);
            float distance = length(lights[i].position - WorldPos);
            float attenuation = 1.0 / (distance * distance);
            radiance = lights[i].color * attenuation;
        }
        vec3 H = normalize(V + L);


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
        Lo += ((kD * albedo / PI + specular) * radiance * NdotL) * shadow ;
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


    vec3 indirect = vec3(0.0);
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    uint rsmSamples = 151;
    float totalWeight = 0.0;
    for (uint i = 0; i < rsmSamples; i++)
    {
        vec2 xi = hammersley(i, rsmSamples);
        float r = xi.x * 0.3;
        float theta = xi.y * (2 * PI);
        vec2 pixelLightUV = projCoords.xy + vec2(r * cos(theta), r * sin(theta));
        float weight = xi.x * xi.x;
        vec3 target_norm = normalize(texture(normal_map, pixelLightUV).xyz);
        vec3 target_world_pos = (texture(position_map, pixelLightUV).xyz);
        vec3 target_flux = texture(flux_map, pixelLightUV).rgb;

        vec3 light_dir = normalize(target_world_pos - FragPos);
        float distance_sq = dot(target_world_pos - FragPos, target_world_pos - FragPos);
        float light_geo = max(0.0, dot(target_norm, -light_dir));
        float receiver_geo = max(0.0, dot(N, light_dir));
        float geometry_term = receiver_geo * light_geo;
        vec3 irradiance = target_flux * geometry_term / distance_sq;
        //        vec3 result = target_world_pos * target_flux * ((max(0.0, dot(target_norm, (FragPos - target_world_pos))) *
        //        max(0.0, dot(N, (target_world_pos - FragPos)))) / pow(length(FragPos - target_world_pos), 4.0));

        indirect += irradiance * weight;
        totalWeight += weight;
    }

    vec3 ambient = (kD * (diffuse) + (specular)) * ao;
    vec3 emission = texture2D(texture_emission, TexCoords).rgb;
    vec3 color = (indirect * 20 * Lo) + emission  + ambient;
//    vec3 color = ((indirect * 200) * Lo) + emission;

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

float shadowCalculation(vec4 fragPos, vec3 n, vec3 l)
{
    vec3 projCoords = fragPos.xyz / fragPos.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
    {
        return 0.0;
    }
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(n, l)), 0.005);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    //    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}