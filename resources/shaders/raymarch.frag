#version 460 core

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D sNormal;
layout (binding = 1) uniform sampler2D gWorld;
layout (binding = 2) uniform sampler2D gDepth;
layout (binding = 3) uniform sampler2D gColor;
layout (binding = 4) uniform sampler2D blue_noise;
layout (location = 0) out vec4 sGI;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    vec2 uv = TexCoords;
    vec3 worldPos = texture(gWorld, TexCoords).rgb;
    vec3 worldNormal = normalize(texture(sNormal, TexCoords).rgb);
    vec3 albedo = texture(gColor, TexCoords).rgb;

    float fragDepth = texture(gDepth, TexCoords).r;

    vec4 viewPosH = view * vec4(worldPos, 1.0);
    vec3 viewPos = viewPosH.xyz;

    vec2 noiseVal = texture(blue_noise, uv).rg;
//    noiseVal += 0.5;

    vec3 rayDirWorld = worldNormal;
    vec3 rayDirView = normalize((view * vec4(rayDirWorld, 0.0)).xyz);

    vec3 accumulated = vec3(0.0);
    int steps = 32;
    float stepSize = 10 / float(steps);
    float currentRayLength = stepSize * 1.5;

    for(int i = 0; i < steps; i++)
    {
        vec3 currentPos = viewPos + rayDirView * currentRayLength;

        vec4 currentPosClip = projection * vec4(currentPos, 1.0);

        if(currentPosClip.w <= 0.0)
        {
            break;
        }

        vec3 currentPosNDC = currentPosClip.xyz / currentPosClip.w;
        currentPosNDC.xy += noiseVal;

//        if(abs(currentPosNDC.x) > 1.0 || abs(currentPosClip.y) > 1.0)
//        {
//            break;
//        }

        vec2 currentUV = currentPosNDC.xy * 0.5 + 0.5;

        float sceneDepth = texture(gDepth, currentUV).r;

        float rayDepth = -currentPos.z;

        float delta = rayDepth - sceneDepth;

        if(delta > 0.01 && delta < 0.1 )
        {
            accumulated = texture(gColor, currentUV).rgb;
            break;
        }
        currentRayLength += stepSize ;
    }

    sGI = vec4(accumulated, 1.0) ;

}