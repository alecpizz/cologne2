#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
//in vec3 FragPos;
//in vec3 WorldPos;
//in mat3 TBN;
//in vec4 FragPosLightSpace;

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
#define ISQRT2 0.707106
#define SQRT2 1.414213
#define MIPMAP_HARDCAP 5.4f
#define DIFFUSE_INDIRECT_FACTOR 0.52f

uniform vec3 camera_pos;
uniform Light lights[MAX_LIGHTS];
uniform int num_lights = 0;
uniform mat4 lightSpaceMatrix;
uniform mat4 view;

layout (binding = 0) uniform sampler2D gPosition;
layout (binding = 1) uniform sampler2D gNormal;
layout (binding = 2) uniform sampler2D gAlbedo;
layout (binding = 3) uniform sampler2D gORM;
layout (binding = 4) uniform sampler2D gEmission;
layout (binding = 5) uniform sampler2DArray shadow_cascades;

layout (binding = 6) uniform samplerCube irradiance_map;
layout (binding = 7) uniform samplerCube prefilter_map;
layout (binding = 8) uniform sampler2D brdf;


//voxel stuff
layout (binding = 9) uniform sampler3D voxel_texture;
layout (binding = 10) uniform sampler2D gNormalFlat;
uniform int voxel_grid_size;
uniform float worldSizeHalf = 150.0f;
uniform float voxel_size = 128;
uniform vec3 world_center = vec3(0.0f);
uniform float sampling_factor = 0.100f;
uniform float distance_offset = 3.9f;
uniform float max_distance = 2.0f;
uniform bool indirect_lighting_active = true;
const int TOTAL_DIFFUSE_CONES = 6;
const vec3 DIFFUSE_CONE_DIRECTIONS[TOTAL_DIFFUSE_CONES] = { vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.5f, 0.866025f), vec3(0.823639f, 0.5f, 0.267617f), vec3(0.509037f, 0.5f, -0.7006629f), vec3(-0.50937f, 0.5f, -0.7006629f), vec3(-0.823639f, 0.5f, 0.267617f) };
const float DIFFUSE_CONE_WEIGHTS[TOTAL_DIFFUSE_CONES] = { 0.25, 0.15, 0.15, 0.15, 0.15, 0.15 };


layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};



uniform float far_plane = 20.0f;
uniform float ao_strength = 0.2;
uniform int has_ao_texture = 0;
uniform float cascadePlaneDistances[4];
uniform int cascadeCount;// number of frusta - 1
vec3 WorldPos;
vec3 WorldNormal;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
float klemenVisibility(vec3 L, vec3 H);
float shadowCalculation(vec3 fragPos, vec3 n, vec3 l);
vec4 texture2D_bilinear(sampler2DArray t, vec3 uv, vec3 texture_size, vec3 texel_size, int layer);

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
    return float(bits) * 2.3283064365386963e-10;// / 0x100000000
}

vec2 hammersley(uint i, uint n)
{
    return vec2(float(i) / float(n), radical_inverse_vdc(i));
}


float rand(vec2 v)
{
    return fract(sin(dot(v, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 Tonemap_ACES(const vec3 x) { // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}


vec3 orthogonal(vec3 u)
{
    u = normalize(u);
    const vec3 v = vec3(0.99146, 0.1164, 0.05832);
    return abs(dot(u, v)) > 0.99999f ? cross(u, vec3(0, 1, 0)) : cross(u, v);
}

vec3 scale_and_bias(const vec3 p) { return 0.5f * p + vec3(0.5f); }

vec4 cone_trace(vec3 from, vec3 direction, float aperture)
{
    float max_dist = 2.0 * worldSizeHalf;
    vec4 accumulated = vec4(0.0);
    float offset = 2.0 * voxel_size;
    float dist = offset + voxel_size;
    while (accumulated.a < 1.0 && dist < max_dist)
    {
        vec3 conePos = from + direction * dist;
        float diameter = 2.0 * aperture * dist;
        float mip = log2(diameter / voxel_size);
        vec3 coords = (conePos - world_center) / worldSizeHalf;
        coords = 0.5 * coords + 0.5;

        vec4 voxel = textureLod(voxel_texture, coords, mip);
        accumulated += (1.0 - accumulated.a) * voxel;
        dist += 0.5 * diameter;
    }
    return accumulated;
}

vec4 indirect_diffuse(vec3 position, vec3 N)
{
    const float aperture = 0.57735;
    vec4 color = vec4(0.0f);
    vec3 up = vec3(0.0, 1.0, 0.0);
    if (abs(dot(N, up)) > 0.999)
    up = vec3(0.0, 0.0, 1.0);

    vec3 T = normalize(up - dot(N, up) * N);
    vec3 B = cross(T, N);

    for (int i = 0; i < TOTAL_DIFFUSE_CONES; i++)
    {
        vec3 direction = T * DIFFUSE_CONE_DIRECTIONS[i].x + B * DIFFUSE_CONE_DIRECTIONS[i].z + N;
        direction = normalize(direction);
        color += DIFFUSE_CONE_WEIGHTS[i] * cone_trace(position, direction, aperture);
    }
    return color;
}

vec4 indirect_specular(vec3 pos, vec3 dir, float roughness)
{
    float aperture = 0.037533;
    aperture = clamp(tan(0.5 * PI * roughness), aperture, 0.5 * PI);
    vec4 specular = cone_trace(pos, dir, aperture);
    return specular;
}

void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    WorldPos = FragPos;
    vec4 albedo_texture = texture2D(gAlbedo, TexCoords).rgba;

    vec3 albedo = pow(albedo_texture.rgb, vec3(2.2));
    vec3 orm = texture2D(gORM, TexCoords).rgb;
    float metallic = orm.r;
    float roughness = orm.g;
    float ao = orm.b + ao_strength;

    vec3 N = texture2D(gNormal, TexCoords).rgb;
    WorldNormal = texture2D(gNormalFlat, TexCoords).rgb;
    vec3 V = normalize(camera_pos - FragPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    vec3 lightDirection = normalize(-lights[0].direction);

    vec4 FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    float shadow = 1.0 - shadowCalculation(FragPos, N, lightDirection);

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
            L = normalize(lights[i].position - FragPos);
            float distance = length(lights[i].position - FragPos);
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
        Lo += ((kD * albedo / PI + specular) * radiance * NdotL) * shadow;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(irradiance_map, N).rgb;
    vec3 diffuse = irradiance * albedo;

    if (indirect_lighting_active)
    {
        vec3 indirect_diffuse = indirect_diffuse(WorldPos, WorldNormal).rgb * 8.0f;
        float factor = min(1, 1 - roughness * 1.0);
        float factor2 = min(1, 1 - metallic * 1.0);
        float factor3 = min(factor, factor2);
        indirect_diffuse *= vec3(factor2);
        indirect_diffuse = max(indirect_diffuse, vec3(0.0));
        indirect_diffuse *= albedo * 1.0;
        vec3 indirect_specular = metallic * indirect_specular(WorldPos, R, roughness).rgb;
        Lo += (kD * indirect_diffuse + indirect_specular) * ao;
    }

    const float MAX_RELFECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilter_map, R, roughness * MAX_RELFECTION_LOD).rgb;
    vec2 brdf = texture(brdf, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);


    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    vec3 ambient = (kD * diffuse + specular) * ao;
    vec3 emission = texture2D(gEmission, TexCoords).rgb;
    vec3 color = Lo + emission;

    color = mix(color, Tonemap_ACES(color), 1.0);
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    color = mix(color, Tonemap_ACES(color), 0.35);
    float alpha = albedo_texture.a;
    color.rgb = color.rgb * alpha;
    FragColor = vec4(color, alpha);
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

float shadowCalculation(vec3 fragPos, vec3 n, vec3 l)
{
    vec4 fragPosViewSpace = view * vec4(fragPos, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    vec4 csmClipSpaceZFar = vec4(cascadePlaneDistances[0],
    cascadePlaneDistances[1], cascadePlaneDistances[2], cascadePlaneDistances[3]);
    vec4 res = step(csmClipSpaceZFar, vec4(depthValue));
    int layer = int(res.x + res.y + res.z + res.w);

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPos, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(n);
    float bias = max(0.05 * (1.0 - dot(normal, l)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == cascadeCount)
    {
        bias *= 1 / (far_plane * biasModifier);
    }
    else
    {
        bias *= 1 / (cascadePlaneDistances[layer] * biasModifier);
    }

    // PCF
    float shadow = 0.0;
    vec2 texture_size = vec2(textureSize(shadow_cascades, 0));
    vec2 texelSize = 1.0 / texture_size;
    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(shadow_cascades, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            //            float pcfDepth2 = texture2D_bilinear(shadow_cascades,
            //                                                 vec3(projCoords.xy + vec2(x, y) * texelSize, layer),
            //                                                 vec3(texture_size.x, texture_size.y, 0.0),
            //                                                 vec3(texelSize, 0.0),
            //                                                 layer).x;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;

    return shadow;
}

vec4 texture2D_bilinear(sampler2DArray t, vec3 uv, vec3 texture_size, vec3 texel_size, int layer)
{
    vec3 f = fract(uv * texture_size);
    uv += (0.5 - f) * texel_size;
    vec4 tl = texture(t, uv, layer);
    vec4 tr = texture(t, uv + vec3(texel_size.x, 0.0, 0.0), layer);
    vec4 bl = texture(t, uv + vec3(0.0, texel_size.y, 0.0));
    vec4 br = texture(t, uv + vec3(texel_size.x, texel_size.y, 0.0), layer);
    vec4 tA = mix(tl, tr, f.x);
    vec4 tB = mix(bl, br, f.x);
    return mix(tA, tB, f.y);
}