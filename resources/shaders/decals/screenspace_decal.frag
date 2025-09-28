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

layout (binding = 5) uniform sampler2D depthTexture;
layout (binding = 6) uniform sampler2D positionTexture;
layout (binding = 7) uniform sampler2D albedoTexture;

uniform mat4 model_inverse;

void main()
{
    vec2 tex_coord = gl_FragCoord.xy / textureSize(positionTexture, 0);
    vec3 world_pos = texture(positionTexture, tex_coord).xyz;

    if(world_pos == vec3(0.0f))
    {
        discard;
    }

    vec4 decal_local_pos = model_inverse * vec4(world_pos, 1.0);
    if(abs(decal_local_pos.x) > 0.5 || abs(decal_local_pos.y) > 0.5 || abs(decal_local_pos.z) > 0.5)
    {
        discard;
    }

    vec2 uv = decal_local_pos.xz + 0.5;
    uv = clamp(uv, 0.0, 1.0);

    vec4 decal_color = texture(decal_albedo, uv); //make this a texture sample!
    if(decal_color.a < 0.1f)
    {
        discard;
    }

    vec4 scene_color = texture(albedoTexture, uv);
    gAlbedo = mix(scene_color, decal_color, decal_color.a);

    gAlbedo = decal_color;
}