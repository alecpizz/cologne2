#version 460 core
#extension GL_ARB_bindless_texture: require
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gORM;
layout (location = 4) out vec3 gEmission;
layout (location = 5) out uint gEntityId;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
flat in uint EntityID;
flat in uint DrawID;
in mat3 TBN;

layout (binding = 0) uniform sampler2D texture_albedo;
layout (binding = 1) uniform sampler2D texture_ao;
layout (binding = 2) uniform sampler2D texture_metallic;
layout (binding = 3) uniform sampler2D texture_roughness;
layout (binding = 4) uniform sampler2D texture_normal;
layout (binding = 5) uniform sampler2D texture_emission;

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    mat4 projection_view_inverse;
    vec4 camera_position;
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

uniform float metallic = -1.0f;
uniform float roughness = -1.0f;
uniform uint entity_id = 0;

vec4 dither_float(vec4 in_pos, vec4 screen_position)
{
    vec2 uv = screen_position.xy * vec2(1600, 900);
    float DITHER_THRESHOLDS[16] =
    {
    1.0 / 17.0, 9.0 / 17.0, 3.0 / 17.0, 11.0 / 17.0,
    13.0 / 17.0, 5.0 / 17.0, 15.0 / 17.0, 7.0 / 17.0,
    4.0 / 17.0, 12.0 / 17.0, 2.0 / 17.0, 10.0 / 17.0,
    16.0 / 17.0, 8.0 / 17.0, 14.0 / 17.0, 6.0 / 17.0
    };
    uint idx = (uint(uv.x) % 4) * 4 + uint(uv.y) % 4;
    return in_pos - DITHER_THRESHOLDS[idx];
}

void main()
{
    Material mat = materials[DrawID];
    //temp branching
    vec4 albedo = texture(texture_albedo, TexCoords).rgba;
    if(mat.albedo != uvec2(0))
    {
        albedo = texture(sampler2D(mat.albedo), TexCoords).rgba;
    }
    if (albedo.a < 0.5)
    {
        discard;
    }
    gEntityId = EntityID;
    gPosition = vec4(FragPos, 1.0);
    vec3 N = texture2D(texture_normal, TexCoords).rgb;
    if(mat.normal != uvec2(0))
    {
        N = texture(sampler2D(mat.normal), TexCoords).rgb;
    }
    N = N * 2.0 - 1.0;
    N = normalize(TBN * N);
    gNormal = vec4(N, 1.0);

    gEmission = texture(texture_emission, TexCoords).rgb;
    if(mat.emission != uvec2(0))
    {
        gEmission = texture(sampler2D(mat.emission), TexCoords).rgb;
    }
    gl_FragDepth = gl_FragCoord.z;
    gORM.r = texture(texture_metallic, TexCoords).b;
    gORM.r *= metallic;
    if(mat.metallic != uvec2(0))
    {
        gORM.r = texture(sampler2D(mat.metallic), TexCoords).b;
        gORM.r *= metallic;
    }

    gORM.g = texture(texture_roughness, TexCoords).g;
    if(mat.roughness != uvec2(0))
    {
        gORM.g = texture(sampler2D(mat.roughness), TexCoords).g;
    }
    gORM.b = texture(texture_ao, TexCoords).r;
    if(mat.ao != uvec2(0))
    {
        gORM.b = texture(sampler2D(mat.ao), TexCoords).b;
    }

    bool allow_dither = false;
    if (allow_dither)
    {
        float distance = distance(camera_position.xyz, gPosition.xyz);
        distance -= 0.0001f;
        vec4 dither = dither_float(vec4(distance, distance, distance, distance), gPosition);
        dither *= albedo.a;
        albedo.a = dither.a;
    }

    gAlbedo.rgba = albedo;
}
