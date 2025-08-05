#version 460 core
#extension GL_ARB_bindless_texture: require
out vec4 FragColor;

in vec2 TexCoords;

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


uniform float time = 0.0f;

layout (binding = 0) uniform sampler2D gPosition;
layout (binding = 1) uniform sampler2D gNormal;
layout (binding = 2) uniform sampler2D gAlbedo;
layout (binding = 3) uniform sampler2D gORM;
layout (binding = 4) uniform sampler2D gEmission;
layout (binding = 5) uniform sampler2DArray shadow_cascades;
layout (binding = 6) uniform sampler2D indirect_texture;
layout (binding = 7) uniform sampler2D bloom_texture;
layout (binding = 8) uniform sampler2D brdf;
layout (binding = 9) uniform samplerCube env_prefilter;

uniform int voxel_grid_size;
uniform float voxel_size = 128;
uniform vec3 world_center = vec3(0.0f);
uniform vec3 grid_max, grid_min;
uniform bool indirect_lighting_active = true;
uniform float fog_density = 0.0225;
uniform vec3 fog_color = vec3(0.22, 0.19, 0.15);
uniform int num_lights = 8;

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    mat4 projection_view_inverse;
    vec4 camera_position;
};

layout (binding = 2, std430) restrict readonly buffer lights_buffer
{
    Light lights[];
};

uniform float far_plane = 20.0f;
uniform float ao_strength = 0.2;

vec3 WorldPos;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
float klemenVisibility(vec3 L, vec3 H);
float point_shadow_calculation(vec3 fragPos, Light light);

const vec3 shadow_offsets[20] = vec3[]
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

vec3 agxDefaultContrastApprox(vec3 x) {
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;

    return + 15.5 * x4 * x2
    - 40.14 * x4 * x
    + 31.96 * x4
    - 6.868 * x2 * x
    + 0.4298 * x2
    + 0.1191 * x
    - 0.00232;
}

vec3 agx(vec3 val)
{
    const mat3 agx_mat = mat3(
    0.842479062253094, 0.0423282422610123, 0.0423756549057051,
    0.0784335999999992, 0.878468636469772, 0.0784336,
    0.0792237451477643, 0.0791661274605434, 0.879142973793104);

    const float min_ev = -12.47393f;
    const float max_ev = 4.026069f;

    // Input transform
    val = agx_mat * val;

    // Log2 space encoding
    val = clamp(log2(val), min_ev, max_ev);
    val = (val - min_ev) / (max_ev - min_ev);

    // Apply sigmoid function approximation
    val = agxDefaultContrastApprox(val);

    return val;
}

vec3 agxEotf(vec3 val)
{
    const mat3 agx_mat_inv = mat3(
    1.19687900512017, -0.0528968517574562, -0.0529716355144438,
    -0.0980208811401368, 1.15190312990417, -0.0980434501171241,
    -0.0990297440797205, -0.0989611768448433, 1.15107367264116);

    // Undo input transform
    val = agx_mat_inv * val;

    // I enabled this line to do linear to srgb in line 180 for all tonemappings.
    // sRGB IEC 61966-2-1 2.2 Exponent Reference EOTF Display
    val = pow(val, vec3(2.2));

    return val;
}

vec3 agxLook(vec3 val)
{
    const vec3 lw = vec3(0.2126, 0.7152, 0.0722);
    float luma = dot(val, lw);

    // Default look
    vec3 offset = vec3(0.0);
    vec3 slope = vec3(1.0);
    vec3 power = vec3(1.0, 1.0, 1.0);
    float sat = 1.0;

    // ASC CDL
    val = pow(val * slope + offset, power);
    return luma + sat * (val - luma);
}

vec3 tonemapping_AgX(vec3 color)
{
    color = agx(color);
    color = agxLook(color);
    color = agxEotf(color);
    return color;
}


vec3 orthogonal(vec3 u)
{
    u = normalize(u);
    const vec3 v = vec3(0.99146, 0.1164, 0.05832);
    return abs(dot(u, v)) > 0.99999f ? cross(u, vec3(0, 1, 0)) : cross(u, v);
}

vec3 scale_and_bias(const vec3 p) { return 0.5f * p + vec3(0.5f); }

vec3 MapRangeToAnOther(vec3 value, vec3 valueMin, vec3 valueMax, vec3 mapMin, vec3 mapMax)
{
    return (value - valueMin) / (valueMax - valueMin) * (mapMax - mapMin) + mapMin;
}

vec3 MapToZeroOne(vec3 value, vec3 rangeMin, vec3 rangeMax)
{
    return MapRangeToAnOther(value, rangeMin, rangeMax, vec3(0.0), vec3(1.0));
}

vec3 noise(vec2 uv, float time)
{
    mat2x3 uvs = mat2x3(uv.xxx, uv.yyy) + mat2x3(vec3(0.0, 0.1, 0.2), vec3(0.0, 0.3, 0.4));
    return fract(sin(uvs * vec2(12.98989, 78.233) * time) * 43856.4533);
}

float dir_shadow_calculation(Light light, vec3 N, vec4 fragPosLightSpace, float bias_scale)
{
    if(light.shadow_map == uvec2(0))
    {
        return 0.0;
    }
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
    {
        return 0.0;
    }
    float currentDepth = projCoords.z;
    float bias = max(bias_scale * (1.0 - dot(N, light.direction.xyz)), bias_scale);
    sampler2DShadow shadow_map = sampler2DShadow(light.shadow_map);
    float shadow = 0.0;
    vec2 texel_size = 1.0f / textureSize(shadow_map, 0);
    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; y++)
        {
            float closestDepth = texture(sampler2DShadow(light.shadow_map), vec3(projCoords.xy + vec2(x, y) * texel_size, currentDepth)).r;
            shadow += currentDepth - bias > closestDepth ? 1.0f : 0.0f;
        }
    }
        shadow /= 25.0f;

    return shadow;
}

void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    WorldPos = FragPos;
    vec4 albedo_texture = texture2D(gAlbedo, TexCoords).rgba;

    vec3 albedo = pow(albedo_texture.rgb, vec3(2.2));
    //    vec3 albedo = albedo_texture.rgb;
    vec3 orm = texture2D(gORM, TexCoords).rgb;
    float metallic = orm.r;
    float roughness = orm.g;
    float ao = orm.b + ao_strength;

    vec3 N = texture2D(gNormal, TexCoords).rgb;
    vec3 V = normalize(camera_position.xyz - FragPos);
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
            radiance = lights[i].color.rgb * lights[i].strength;
            shadow = 1.0 - dir_shadow_calculation(lights[i], N, lights[i].light_space_matrix * vec4(FragPos, 1.0), 0.0005f);
        }
        else if (lights[i].type == POINT)
        {
            L = normalize(lights[i].position.xyz - FragPos);
            float distance = length(lights[i].position.xyz - FragPos);
            float dist_range = distance / lights[i].radius;
            float falloff = pow(dist_range, 2.0f);
            float smoothing = pow(max(0.0, 1.0 - falloff), 2.0f);
            float attenuation = smoothing / (distance * distance + 1.0f);
            radiance = lights[i].color.rgb * lights[i].strength * attenuation;
            shadow = point_shadow_calculation(FragPos, lights[i]);

        }
        else if (lights[i].type == SPOT)
        {
            L = normalize(lights[i].position.xyz - FragPos);
            float distance = length(lights[i].position.xyz - FragPos);
            float dist_range = distance / lights[i].radius;
            float falloff = pow(dist_range, 2.0f);
            float smoothing = pow(max(0.0, 1.0 - falloff), 2.0f);
            float attenuation = smoothing / (distance * distance + 1.0f);

            float theta = dot(L, -lights[i].direction.xyz);
            float epsilon = lights[i].inner_cutoff - lights[i].outer_cutoff;
            float intensity = smoothstep(0.0, 1.0, (theta - lights[i].outer_cutoff) / epsilon);
            radiance = lights[i].color.rgb * lights[i].strength * attenuation * intensity;
            shadow = 1.0 - dir_shadow_calculation(lights[i], N, lights[i].light_space_matrix * vec4(FragPos, 1.0), 0.00025f);
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

    vec3 indirect_light = vec3(0.0);
    vec3 emission = texture2D(gEmission, TexCoords).rgb;
    if (indirect_lighting_active)
    {
        indirect_light = texture(indirect_texture, TexCoords).rgb;
//        float factor = min(1, roughness);
//        indirect_light *= (0.85) * vec3(factor);
//        indirect_light = max(indirect_light, vec3(0));
        indirect_light *= albedo;
        indirect_light *= 0.235f;
        //        indirect_light = max(indirect_light, emission);
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(env_prefilter, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdf, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 color = Lo + indirect_light ;
    if(N.x == 0.0f && N.z == 0.0f && N.y == 0.0f)
    {
        color = albedo;
    }

    float dist = length(FragPos - camera_position.xyz);
    float fog_factor = 1.0 / exp((dist * fog_density) * (dist * fog_density));
    color = mix(fog_color, color, fog_factor);

    color += texture(bloom_texture, TexCoords).rgb;
    color = mix(color, tonemapping_AgX(color), 1.0);
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    color += emission;

    vec2 uv = TexCoords;
    vec2 coord = gl_FragCoord.xy;
    vec2 output_size = vec2(textureSize(gAlbedo, 0));
    vec2 outside = modf(uv * output_size, coord);
    vec3 noise00 = noise(coord / output_size, time);
    vec3 noise01 = noise((coord + vec2(0, 1)) / output_size, time);
    vec3 noise10 = noise((coord + vec2(1, 0)) / output_size, time);
    vec3 noise11 = noise((coord + vec2(1, 1)) / output_size, time);
    vec3 noise = mix(mix(noise00, noise01, outside.y),
                     mix(noise10, noise11, outside.y), outside.x) * vec3(0.7, 0.6, 0.8);
    float speed = 15.0f;
    float x = rand(uv + rand(vec2(int(time * speed), int(-time * speed))));
    float noise_fact = 0.035;
    color = color + (x * -noise_fact) + (noise_fact / 2);

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

float point_shadow_calculation(vec3 fragPos, Light light)
{
    if (light.shadow_map == uvec2(0.0))
    {
        return 1.0f;
    }
    vec3 lightPos = light.position.xyz;
    vec3 frag_to_light = fragPos - lightPos;
    float bias = 0.005;
    float disk_radius = 0.05;
    float shadow = 0.0f;
    for(int i = 0; i < 20; i++)
    {
        vec3 samplePos = (frag_to_light + shadow_offsets[i] * disk_radius);
        float depth = get_light_space_depth(1.0f, light.radius, samplePos * (1.0 - bias));
        shadow += texture(samplerCubeShadow(light.shadow_map), vec4(samplePos, depth));
    }
    shadow /= 20;
    return shadow;
}
