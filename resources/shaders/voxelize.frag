#version 460 core
#extension GL_NV_shader_atomic_fp16_vector: require
#extension GL_NV_gpu_shader5: require
#extension GL_ARB_bindless_texture : require
layout (binding = 0) uniform sampler2D texture_albedo;
layout (binding = 1) uniform sampler2D texture_ao;
layout (binding = 2) uniform sampler2D texture_metallic;
layout (binding = 3) uniform sampler2D texture_roughness;
layout (binding = 4) uniform sampler2D texture_normal;
layout (binding = 5) uniform sampler2D texture_emission;
layout (RGBA16F, binding = 6) uniform image3D texture_voxel;
layout (binding = 7) uniform sampler2DArray shadow_cascades;
layout (binding = 8) uniform sampler2D dir_shadow;
uniform vec3 voxel_size;

layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

uniform float far_plane = 20.0f;
uniform float cascadePlaneDistances[4];
uniform int cascadeCount;// number of frusta - 1

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
};

#define MAX_LIGHTS 8
#define PI 3.1415926535897932384626433832795
#define DIRECTIONAL 0
#define POINT 1

layout (binding = 2, std430) restrict readonly buffer lights_buffer
{
    Light lights[];
};
uniform mat4 view;


in vec2 TexCoords;
in vec4 FragPos;
in mat3 TBN;
in vec4 FragPosLightSpace;

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

float shadow_calculation2(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
    {
        return 0.0;
    }
    float closestDepth = texture(dir_shadow, projCoords.xy).r;
    float currentDepth = projCoords.z;
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

float point_shadow_calc(vec3 fragPos, vec3 lightPos, samplerCubeShadow shadow_map)
{
    vec3 frag_to_light = fragPos - lightPos;
    float bias = 0.02f;
    float current_depth = get_light_space_depth(1.0f, 20.0f, frag_to_light );
    float shadow = texture(shadow_map, vec4(frag_to_light, current_depth ));
    return shadow;
}


vec4 pbr()
{
    vec4 albedo_texture = texture2D(texture_albedo, TexCoords);
//    albedo = albedo_texture.rgb;
    vec3 albedo = pow(albedo_texture.rgb, vec3(2.2));
    float metallic = texture2D(texture_metallic, TexCoords).r;
    float roughness = texture2D(texture_roughness, TexCoords).g;
    float ao = texture2D(texture_ao, TexCoords).b + 0.2;

    vec3 N = texture2D(texture_normal, TexCoords).rgb;
    N = N * 2.0 - 1.0;
    N = normalize(TBN * N);

    vec3 V = normalize(-FragPos.xyz);
    vec3 R = reflect(-V, N);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < lights.length(); i++)
    {
        if(lights[i].enabled == 0)
        {
            continue;
        }
        float shadow = 1.0f;
        vec3 L = vec3(0.0);
        vec3 radiance = vec3(0.0);
        if (lights[i].type == DIRECTIONAL)
        {
            L = normalize(-lights[i].direction.xyz);
            radiance = lights[i].color.rgb * lights[i].strength;
            shadow = 1.0 - shadow_calculation2(FragPosLightSpace);
        }
        else if (lights[i].type == POINT)
        {
            vec3 light_pos_voxel_space = lights[i].position.xyz * voxel_size;
            L = normalize(light_pos_voxel_space - FragPos.xyz);
            float distance = length(light_pos_voxel_space - FragPos.xyz);
            float attenuation = 1.0 / (distance * distance);
            shadow = point_shadow_calc(FragPos.xyz, light_pos_voxel_space, samplerCubeShadow(lights[i].shadow_map));
            radiance = lights[i].color.rgb * lights[i].strength * attenuation * 0.1f;
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

    vec3 ambient = vec3(0.02) * albedo;
    vec3 emission = texture2D(texture_emission, TexCoords).rgb;
    vec3 color = Lo + ambient + emission;
    return vec4(color, 1.0);
}

void main()
{
    if (!is_inside_clipspace(FragPos.xyz))
    {
        return;
    }


    vec4 color = pbr();

    vec3 voxelgrid_tex_pos = from_clipspace_to_texcoords(FragPos.xyz);
    ivec3 voxelgrid_resolution = imageSize(texture_voxel);
    //    imageStore(texture_voxel, ivec3(voxelgrid_resolution * voxelgrid_tex_pos), color);
    imageAtomicMax(texture_voxel, ivec3(voxelgrid_resolution * voxelgrid_tex_pos), f16vec4(color));
}