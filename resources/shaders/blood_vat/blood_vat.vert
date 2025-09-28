#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

layout (binding = 0) uniform sampler2D position_texture;
layout (binding = 1) uniform sampler2D normal_texture;
uniform mat4 model;
uniform float time = 1.0f;
uniform vec3 height_offset;
out vec4 WorldPos;
out vec3 Normal;

layout (binding = 1, std430) restrict readonly buffer viewportdata
{
    mat4 projection;
    mat4 view;
    mat4 view_inverse;
    mat4 projection_view;
    mat4 projection_view_inverse;
    vec4 camera_position;
};

void main()
{
    mat4 projectionView = projection_view;
    mat4 inverseView = view_inverse;

    mat4 modelMatrix = model;
    mat4 inverseModelMatrix = inverse(modelMatrix);
    mat4 normalMatrix = transpose(inverseModelMatrix);

    int u_NumOfFrames = 81;
    int u_Speed = 35;

    float bounding_min = 144;
    float bounding_max = 144;
    float currentSpeed = 1.0f / (u_NumOfFrames / u_Speed);
    float timeInFrames = ((ceil(fract(-time * currentSpeed) * u_NumOfFrames)) / u_NumOfFrames) + (1.0 / u_NumOfFrames);

    vec3 v = position;

    timeInFrames = 0.0;
    timeInFrames = time;

    vec2 TexCoord = vec2(uv.x, (timeInFrames + uv.y));
    TexCoord = clamp(TexCoord, 0, 1);

    vec4 texturePos = textureLod(position_texture, TexCoord, 0);
    texturePos.xyz += height_offset.xyz;
    vec4 textureNorm = textureLod(normal_texture, TexCoord, 0);

    Normal = textureNorm.xzy * 2.0 - 1.0;
    Normal = normalize((normalMatrix * vec4(Normal, 0)).xyz);

    WorldPos = modelMatrix * vec4(texturePos.xzy, 1.0);

    gl_Position =  projectionView * WorldPos;
}

