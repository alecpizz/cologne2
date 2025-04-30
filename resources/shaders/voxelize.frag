#version 460 core
#extension GL_NV_shader_atomic_fp16_vector: require
#extension GL_NV_gpu_shader5: require
layout (binding = 0) uniform sampler2D texture_albedo;
layout (binding = 1) uniform sampler2D texture_ao;
layout (binding = 2) uniform sampler2D texture_metallic;
layout (binding = 3) uniform sampler2D texture_roughness;
layout (binding = 4) uniform sampler2D texture_normal;
layout (binding = 5) uniform sampler2D texture_emission;
layout (RGBA16F, binding = 6) uniform image3D texture_voxel;
layout (binding = 7) uniform sampler2DArray shadow_cascades;
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

uniform Light lights[MAX_LIGHTS];
uniform int num_lights = 0;
uniform mat4 view;


in vec2 f_tex_coords;
in vec3 f_voxel_pos;
in vec4 f_shadow_coords;
in mat3 f_TBN;

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

float shadow_calculation(vec3 p, vec3 n, vec3 l)
{
    vec4 fragPosViewSpace = vec4(p, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    vec4 csmClipSpaceZFar = vec4(cascadePlaneDistances[0],
    cascadePlaneDistances[1], cascadePlaneDistances[2], cascadePlaneDistances[3]);
    vec4 res = step(csmClipSpaceZFar, vec4(depthValue));
    int layer = int(res.x + res.y + res.z + res.w);

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(p, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

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

    float shadow = 0.0;
    vec2 texture_size = vec2(textureSize(shadow_cascades, 0));
    vec2 texelSize = 1.0 / texture_size;
    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(shadow_cascades, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;

    return shadow;
}

vec3 diffuse(Light light, vec3 albedo, vec3 sampleToLight, vec3 N)
{
    float dist = length(sampleToLight);
    if (light.type == DIRECTIONAL)
    {
        vec3 diffuse = light.color * dot(normalize(N), -light.direction) * albedo;
        return diffuse;
    }
    else if (light.type == POINT)
    {
        sampleToLight = sampleToLight / dist;
        float cosTheta = dot(normalize(N), (sampleToLight));
        if (cosTheta > 0.0)
        {
            vec3 diffuse = light.color * cosTheta * albedo;
            float radiance = 1.0 / (dist * dist);
            return diffuse * radiance;
        }
    }
    return vec3(0.0);
}

vec4 pbr()
{
    vec3 FragPos = f_voxel_pos;
    vec4 albedo_texture = texture2D(texture_albedo, f_tex_coords);
    vec3 albedo = albedo_texture.rgb;
//    vec3 albedo = pow(albedo_texture.rgb, vec3(2.2));
    vec3 orm;
    float metallic = texture2D(texture_metallic, f_tex_coords).r;
    float roughness = texture2D(texture_roughness, f_tex_coords).g;
    float ao = texture2D(texture_ao, f_tex_coords).b + 0.2;

    vec3 N = texture2D(texture_normal, f_tex_coords).rgb;
    N = N * 2.0 - 1.0;
    N = normalize(f_TBN * N);

    vec3 V = normalize(-f_voxel_pos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    float shadow = 1.0 - shadow_calculation(FragPos, N, -lights[0].direction);
    vec3 color = vec3(0.0f);
    color += diffuse(lights[0], albedo, lights[0].position - FragPos, N) * shadow;
    for (int i = 1; i < num_lights; i++)
    {
//        color += diffuse(lights[i], albedo, lights[i].position - FragPos, N);
    }

//    vec3 ambient = vec3(0.02) * albedo;
//    color += ambient;
    return vec4(color, 1.0);
}

void main()
{
    if (!is_inside_clipspace(f_voxel_pos))
    {
        return;
    }


    vec4 color = pbr();

    vec3 voxelgrid_tex_pos = from_clipspace_to_texcoords(f_voxel_pos);
    ivec3 voxelgrid_resolution = imageSize(texture_voxel);
    //    imageStore(texture_voxel, ivec3(voxelgrid_resolution * voxelgrid_tex_pos), color);
    imageAtomicMax(texture_voxel, ivec3(voxelgrid_resolution * voxelgrid_tex_pos), f16vec4(color));
}