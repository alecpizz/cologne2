#version 460 core

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D sNormal;
layout (binding = 1) uniform sampler2D gWorld;
layout (binding = 2) uniform sampler2D gDepth;
layout (binding = 3) uniform sampler2D gColor;
layout (location = 0) out vec4 sGI;
uniform mat4 view;
uniform mat4 projection;

#define hash(p)  fract(sin(dot(p, vec2(11.9898, 78.233))) * 43758.5453

float B(vec2 U)
{
    float v = 0.;
    for (int k=0; k<9; k++)
    v += hash( U + vec2(k%3-1,k/3-1) ));
    //return       1.125*hash(U)- v/8.  + .5; // some overbound, closer contrast
    return .9 *(1.125*hash(U)- v/8.) + .5);//
    //return .75*( 1.125*hash(U)- v/8.) + .5; // very slight overbound
    //return .65*( 1.125*hash(U)- v/8.) + .5; // dimmed, but histo just fit without clamp. flat up to .5 +- .23
}

vec4 raymarch(sampler2D depthTex, mat4 projmat, vec3 rayDir, int steps, vec3 startPos, vec3 screenPos, vec2 uvCoord, float stepSize, float maxDist)
{
    float depth = 0.0;
    for(int i = 0; i < steps; i++)
    {
        vec3 currentPos = startPos + rayDir * float(i);
        vec4 projPosH = projmat * vec4(currentPos, 1.0);
        vec3 projectedPos = projPosH.xyz / projPosH.w;
        vec2 new_uv = uvCoord;

        if(new_uv.x >= 0.0 && new_uv.x <= 1.0 && new_uv.y >= 0.0 && new_uv.y <= 1.0)
        {
            float sceneDepth = texture(depthTex, new_uv).r;
            float linearDepth = -startPos.z + depth;

            if(linearDepth < sceneDepth)
            {
                return vec4(new_uv.x, new_uv.y, 0.0, 1.0);
            }
        }
        depth += stepSize;
        if(depth > maxDist)
        {
            break;
        }
    }
    return vec4(0.0);
}

void main()
{
    vec2 uv = TexCoords;
    vec3 worldPos = texture(gWorld, TexCoords).rgb;
    vec3 worldNormal = normalize(texture(sNormal, TexCoords).rgb);
    vec3 albedo = texture(gColor, TexCoords).rgb;

    float fragDepth = texture(gDepth, TexCoords).r;

    vec4 viewPosH = view * vec4(worldPos, 1.0);
    vec3 viewPos = viewPosH.xyz;

    vec2 noiseVal = vec2(B(uv), B(uv * 9));

    vec3 rayDirWorld = worldNormal;
    vec3 rayDirView = normalize((view * vec4(rayDirWorld, 0.0)).xyz);

    vec3 accumulated = vec3(0.0);
    int steps = 12;
    float stepSize = 1.0 / float(steps);
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

        if(abs(currentPosNDC.x) > 1.0 || abs(currentPosClip.y) > 1.0)
        {
            break;
        }

        vec2 currentUV = currentPosNDC.xy * 0.5 + 0.5;

        float sceneDepth = texture(gDepth, currentUV).r;

        float rayDepth = -currentPos.z;

        float delta = rayDepth - sceneDepth;

        if(delta > 0.01 && delta < 0.1 && sceneDepth < 300 * 0.999)
        {
            accumulated = texture(gColor, currentUV).rgb;
            break;
        }
        currentRayLength += stepSize * 1.0;
    }

    sGI = vec4(accumulated, 1.0);

}