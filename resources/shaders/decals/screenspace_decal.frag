#version 460 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gORM;
layout (location = 4) out vec3 gEmission;
layout (location = 5) out uint gEntityId;

layout (binding = 0) uniform sampler2D decal_albedo;
layout (binding = 1) uniform sampler2D decal_normal;
layout (binding = 2) uniform sampler2D decal_orm;
layout (binding = 3) uniform sampler2D decal_emission;

layout (binding = 4) uniform sampler2D ormTexture;
layout (binding = 5) uniform sampler2D depthTexture;
layout (binding = 6) uniform sampler2D positionTexture;
layout (binding = 7) uniform sampler2D albedoTexture;
layout (binding = 8) uniform sampler2D normalTexture;

uniform mat4 model_inverse;
uniform mat4 model_normal;
uniform vec4 tint_color;

void main()
{
    vec2 tex_coord = gl_FragCoord.xy / textureSize(positionTexture, 0);
    vec3 world_pos = texture(positionTexture, tex_coord).xyz;
    vec4 world_normal = texture(normalTexture, tex_coord);
    if(world_pos == vec3(0.0f))
    {
        discard;
    }

    float depth = texture(depthTexture, tex_coord).r;
    if(depth < 0.0001)
    {
        discard;
    }

    vec4 decal_local_pos = model_inverse * vec4(world_pos, 1.0);
    if(abs(decal_local_pos.x) > 0.5 || abs(decal_local_pos.y) > 0.5 || abs(decal_local_pos.z) > 0.5)
    {
        discard;
    }


    vec3 local_normal = normalize(model_normal * world_normal).xyz;

    vec3 blend_weights = abs(local_normal.xyz);
    blend_weights = normalize(max(blend_weights, 0.00001));
    blend_weights /= vec3(blend_weights.x + blend_weights.y + blend_weights.z);

    vec2 uv_x = decal_local_pos.yz + 0.5;
    vec2 uv_y = decal_local_pos.xz + 0.5;
    vec2 uv_z = decal_local_pos.xy + 0.5;
    uv_x = clamp(uv_x, 0, 1);
    uv_y = clamp(uv_y, 0, 1);
    uv_z = clamp(uv_z, 0, 1);

    vec4 decal_color = texture(decal_albedo, uv_y);
   // vec4 decal_color = decal_color_x * blend_weights.x + decal_color_y * blend_weights.y + decal_color_z * blend_weights.z;

    vec4 normal_x = texture(decal_normal, uv_x);
    vec4 normal_y = texture(decal_normal, uv_z);
    vec4 normal_z = texture(decal_normal, uv_z);
    vec4 decal_norm = texture(decal_normal, uv_y);

    vec4 scene_color = texture(albedoTexture, tex_coord);
    vec4 scene_orm = texture(ormTexture, tex_coord);
    vec3 decal_orm = vec3(1.0, 0.015, 0.54);
    vec4 final_decal_color = decal_color * tint_color;
    gORM = decal_orm;
    gAlbedo = final_decal_color;
    gNormal = decal_norm;
    gPosition = vec4(world_pos, 1.0);
    gEmission = vec3(0.0, 0.0, 0.0);
}