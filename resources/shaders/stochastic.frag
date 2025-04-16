#version 460 core

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D gNormal;
layout (location = 0) out vec4 sNormal;

uniform int frame;
uniform vec2 screen_resolution;

vec2 random2(vec3 st){
    vec2 S = vec2(dot(st, vec3(127.1, 311.7, 783.089)),
    dot(st, vec3(269.5, 183.3, 173.542)));
    return fract(sin(S)*43758.5453123);
}

vec3 get_cos_hemisphere_sample(float rand1, float rand2, vec3 n)
{
    vec2 randVal = vec2(rand1, rand2);
    vec3 bitangent = normalize(cross(n, vec3(0.0, 1.0, 0.0)));
    if(length(bitangent) < 0.0001)
    {
        bitangent = normalize(cross(n, vec3(1.0, 0.0, 0.0)));
    }
    vec3 tangent = cross(bitangent, n);
    float r = sqrt(randVal.x);
    float phi = 2.0 * 3.14159265 * randVal.y;
    return tangent * (r * cos(phi)) + bitangent * (r * sin(phi)) + n * sqrt(max(0.0, 1.0 - randVal.x));
}

float IGN(vec2 uv) { return fract(52.9829189 * fract(dot(uv, vec2(0.06711056, 0.00583715)))); }

float EvalIGN(vec2 uv)
{
    uint f = uint(frame);

    //frame += WellonsHash2(WeylHash(uvec2(uv)/4u)) % 4u;

    if((f & 2u) != 0u) uv = vec2(-uv.y, uv.x);
    if((f & 1u) != 0u) uv.x = -uv.x;

    //return fract(52.9829189 * fract(dot(uv, vec2(0.06711056, 0.00583715))) + float(frame)*0.41421356);
    //return fract(52.9829189 * fract(dot(uv, vec2(0.06711056, 0.00583715))));
    //return fract(IGN(uv)+float(frame)*0.41421356*1.0);

    // http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/#dither
    return fract(uv.x*0.7548776662 + uv.y*0.56984029 + float(f)*0.41421356*1.0);
}

vec4 create_custom_normals()
{
    vec2 uv = TexCoords;
    vec2 pos = uv * screen_resolution.xy;
    vec4 normal = texture(gNormal, uv);
    float normalLength = length(normal);
    float noise = EvalIGN(pos);
    vec3 stochastic_normal = get_cos_hemisphere_sample(noise, noise, normal.rgb);
    return normalize(vec4(stochastic_normal, 1.0));
}

void main()
{
    vec4 result = create_custom_normals();
    sNormal = result;

    //turn this shit stochastic
}