#version 460 core
#extension GL_NV_shader_atomic_fp16_vector: require
#extension GL_NV_gpu_shader5: require
#extension GL_ARB_bindless_texture: require

layout (RGBA16F, binding = 6) uniform image3D texture_voxel;
layout (r32ui, binding = 7) restrict uniform uimage3D texture_voxel_normal;

uniform vec3 grid_min;
uniform vec3 grid_max;

struct Light
{
    vec4 direction;
    vec4 position;
    vec4 color;
    float strength;
    float radius;
    int type;
    int enabled;
    uvec2 shadow_map;
    float outer_cutoff;
    float inner_cutoff;
    mat4 light_space_matrix;
};

#define MAX_LIGHTS 8
#define PI 3.1415926535897932384626433832795
#define DIRECTIONAL 0
#define POINT 1
#define SPOT 2

layout (binding = 2, std430) restrict readonly buffer lights_buffer
{
    Light lights[];
};

struct Material
{
    uvec2 albedo;
    uvec2 normal;
    uvec2 metallic;
    uvec2 roughness;
    uvec2 ao;
    uvec2 emission;
    float roughness_mod;
    float metallic_mod;
};

layout (binding = 5, std430) restrict readonly buffer materialdata
{
    Material materials[];
};

uniform int num_lights = 8;

in vec2 TexCoords;
in vec4 FragPos;
in mat3 TBN;
in vec3 Normal;
in vec4 FragPosLightSpace[8];
flat in uint DrawID;

bool is_inside_clipspace(const vec3 p)
{
    return abs(p.x) < 1 && abs(p.y) < 1 && abs(p.z) < 1;
}

vec3 from_clipspace_to_texcoords(vec3 p)
{
    return 0.5f * p + vec3(0.5f);
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

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float dir_shadow_calc(Light light, vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
    {
        return 0.0;
    }
    float currentDepth = projCoords.z;
    float closestDepth = texture(sampler2D(light.shadow_map), projCoords.xy).r;
    float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
    return shadow;
}

float GetAttenuationFactor(float distSq, float lightRadius)
{
    lightRadius = max(lightRadius, 0.0001);
    distSq = max(distSq, 0.0001);

    float factor = (lightRadius * lightRadius) / distSq;

    return factor;
}

float get_log_depth(float near, float far, float distance)
{
    float depth = (1.0 / distance - 1.0 / near) / (1.0 / far - 1.0 / near);
    return depth;
}

float get_light_space_depth(float near, float far, vec3 light_to_sample)
{
    float dist = max(abs(light_to_sample.x), max(abs(light_to_sample.y), abs(light_to_sample.z)));
    float depth = get_log_depth(near, far, dist);
    return depth;
}

float point_shadow_calc(vec3 fragPos, Light light)
{
    if(light.shadow_map == uvec2(0.0))
    {
        return 1.0f;
    }
    vec3 lightPos = light.position.xyz;
    vec3 frag_to_light = fragPos - lightPos;
    float bias = 0.02f;
    float far_plane = (light.radius);
    float current_depth = get_light_space_depth(1.0f, far_plane, frag_to_light);
    float shadow = texture(samplerCubeShadow(light.shadow_map), vec4(frag_to_light, current_depth));
    return shadow;
}

vec3 Tonemap_ACES(const vec3 x)
{ // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec4 pbr()
{
    Material mat = materials[DrawID];
    vec4 albedo_texture = texture2D(sampler2D(mat.albedo), TexCoords);
    vec3 albedo = pow(albedo_texture.rgb, vec3(2.2));
    float metallic;
    float roughness;
    if(mat.metallic == uvec2(0))
    {
        metallic = mat.metallic_mod;
    }
    else
    {
        metallic = texture2D(sampler2D(mat.metallic), TexCoords).r;
        metallic *= mat.metallic_mod;
    }

    if(mat.roughness == uvec2(0))
    {
        roughness = mat.roughness_mod;
    }
    else
    {
        roughness = texture2D(sampler2D(mat.roughness), TexCoords).g;
        roughness *= mat.roughness_mod;
    }
    float ao = texture2D(sampler2D(mat.ao), TexCoords).b + 0.2;

    vec3 N;
    if(mat.normal == vec2(0))
    {
        N = Normal;
    }
    else
    {
        N = texture2D(sampler2D(mat.normal), TexCoords).rgb;
    }
    N = N * 2.0 - 1.0;
    N = normalize(TBN * N);

    vec3 V = normalize(-FragPos.xyz);
    vec3 R = reflect(-V, N);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < num_lights; i++)
    {
        if (lights[i].enabled == 0)
        {
            continue;
        }
        float shadow = 1.0f;
        vec3 L = vec3(0.0);
        vec3 radiance = vec3(0.0);
        if (lights[i].type == DIRECTIONAL)
        {
            L = normalize(-lights[i].direction.xyz);
            radiance = lights[i].color.rgb * lights[i].strength * 4.0f;
            shadow = 1.0 - dir_shadow_calc(lights[i], FragPosLightSpace[i]);
        }
        else if (lights[i].type == POINT)
        {
            L = normalize(lights[i].position.xyz - FragPos.xyz);
            float distance = length(lights[i].position.xyz - FragPos.xyz);
            float dist_range = distance / (lights[i].radius);
            float falloff = pow(dist_range, 2.0f);
            float smoothing = pow(max(0.0, 1.0 - falloff), 2.0f);
            float attenuation = smoothing / (distance * distance + 1.0f);
            shadow = point_shadow_calc(FragPos.xyz, lights[i]);
            radiance = lights[i].color.rgb * lights[i].strength * attenuation;
        }
        else if(lights[i].type == SPOT)
        {
            L = normalize(lights[i].position.xyz - FragPos.xyz);
            float distance = length(lights[i].position.xyz - FragPos.xyz);
            float dist_range = distance / lights[i].radius;
            float falloff = pow(dist_range, 2.0f);
            float smoothing = pow(max(0.0, 1.0 - falloff), 2.0f);
            float attenuation = smoothing / (distance * distance + 1.0f);

            float theta = dot(L, -lights[i].direction.xyz);
            float epsilon   = lights[i].inner_cutoff - lights[i].outer_cutoff;
            float intensity = smoothstep(0.0, 1.0, (theta - lights[i].outer_cutoff) / epsilon);
            radiance = lights[i].color.rgb * lights[i].strength * attenuation * intensity;
            shadow = 1.0 - dir_shadow_calc(lights[i], FragPosLightSpace[i]);
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

    const vec3 ambient_light_color = vec3(1.0, 0.98, 0.94);
    const float ambient_strength = 0.05f;
    vec3 ambient_color = albedo * ambient_light_color;
    vec3 ambient_lighting = ambient_color * ambient_strength;
    vec3 emission = texture2D(sampler2D(mat.emission), TexCoords).rgb;
    vec3 color = Lo  + ambient_lighting + emission;

    //    color = pow(color, vec3(1.0 / 2.2));
    //    color = color / (color + vec3(1.0));
    //    color = mix(color, Tonemap_ACES(color), 1.0);
    //    color = mix(color, Tonemap_ACES(color)    , 0.35);
    //    color.r = max(color.r, emission.r);
    //    color.g = max(color.g, emission.g);
    //    color.b = max(color.b, emission.b);
    return vec4(color, 1.0);
}

vec3 MapRangeToAnOther(vec3 value, vec3 valueMin, vec3 valueMax, vec3 mapMin, vec3 mapMax)
{
    return (value - valueMin) / (valueMax - valueMin) * (mapMax - mapMin) + mapMin;
}

vec3 MapToZeroOne(vec3 value, vec3 rangeMin, vec3 rangeMax)
{
    return MapRangeToAnOther(value, rangeMin, rangeMax, vec3(0.0), vec3(1.0));
}


ivec3 WorldSpaceToVoxelImageSpace(vec3 worldPos)
{
    vec3 uvw = MapToZeroOne(worldPos, grid_min, grid_max);
    ivec3 voxelPos = ivec3(uvw * imageSize(texture_voxel));
    return voxelPos;
}

uint packSnorm2x8(vec2 v) { uvec2 d = uvec2(round(127.5 + v * 127.5)); return d.x | (d.y << 8u); }

vec2 msign(vec2 v)
{
    return vec2((v.x >= 0.0) ? 1.0 : -1.0,
    (v.y >= 0.0) ? 1.0 : -1.0);
}


void main()
{
    ivec3 voxel_pos = WorldSpaceToVoxelImageSpace(FragPos.xyz);
    vec4 color = pbr();
//    color = color / (color + vec4(1.0));
//    color = pow(color, vec4(1.0 / 2.2));
    imageAtomicMax(texture_voxel, voxel_pos, f16vec4(color));
    //    imageAtomicMax(texture_voxel_normal, voxel_pos, (normal));
    //    imageStore(texture_voxel_normal, voxel_pos, uvec4(normal));
}