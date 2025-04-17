#version 460 core

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D gDepth;
layout (location = 0) out vec4 sNormal;

uniform int frame;
uniform vec2 screen_resolution;
uniform mat4 inverse_projection;
uniform mat4 inverse_view;

vec2 random2(vec3 st){
    vec2 S = vec2(dot(st, vec3(127.1, 311.7, 783.089)),
    dot(st, vec3(269.5, 183.3, 173.542)));
    return fract(sin(S)*43758.5453123);
}

vec3 get_cos_hemisphere_sample(float rand1, float rand2, vec3 n)
{
    vec2 randVal = vec2(rand1, rand2);
    vec3 bitangent = normalize(cross(n, vec3(0.0, 1.0, 0.0)));
    if (length(bitangent) < 0.0001)
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

    if ((f & 2u) != 0u) uv = vec2(-uv.y, uv.x);
    if ((f & 1u) != 0u) uv.x = -uv.x;

    //return fract(52.9829189 * fract(dot(uv, vec2(0.06711056, 0.00583715))) + float(frame)*0.41421356);
    //return fract(52.9829189 * fract(dot(uv, vec2(0.06711056, 0.00583715))));
    //return fract(IGN(uv)+float(frame)*0.41421356*1.0);

    // http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/#dither
    return fract(uv.x*0.7548776662 + uv.y*0.56984029 + float(f)*0.41421356*1.0);
}

float linearize_depth(float depth)
{
    float ndc = depth * 2.0 - 1.0;

    float linear = inverse(inverse_projection)[3][2] / (inverse(inverse_projection)[2][2] + ndc);
    return linear;
}

vec3 worldpos_from_depth(vec2 uv, float depth)
{
    vec2 ndc = uv * 2.0 - 1.0;
    float viewZ = -depth;
    float viewX = ndc.x * viewZ / (-inverse(inverse_projection)[0][0]);
    float viewY = ndc.y * viewZ / (-inverse(inverse_projection)[0][0]);
    return vec3(viewX, viewY, viewZ);
}


//vec4 compute_normal_from_depth(ivec2 p)
//{
//    //    float c0 = texelFetch(gDepth, p, 0).w;
//    //    float l2 = texelFetch(gDepth, p-ivec2(2, 0), 0).w;
//    //    float l1 = texelFetch(gDepth, p-ivec2(1, 0), 0).w;
//    //    float r1 = texelFetch(gDepth, p+ivec2(1, 0), 0).w;
//    //    float r2 = texelFetch(gDepth, p+ivec2(2, 0), 0).w;
//    //    float b2 = texelFetch(gDepth, p-ivec2(0, 2), 0).w;
//    //    float b1 = texelFetch(gDepth, p-ivec2(0, 1), 0).w;
//    //    float t1 = texelFetch(gDepth, p+ivec2(0, 1), 0).w;
//    //    float t2 = texelFetch(gDepth, p+ivec2(0, 2), 0).w;
//    //
//    //    float dl = abs(l1*l2/(2.0*l2-l1)-c0);
//    //    float dr = abs(r1*r2/(2.0*r2-r1)-c0);
//    //    float db = abs(b1*b2/(2.0*b2-b1)-c0);
//    //    float dt = abs(t1*t2/(2.0*t2-t1)-c0);
//    //
//    //    vec3 ce = worldpos_from_depth(p, c0);
//    //    vec3 dpdx = (dl<dr) ?  ce-worldpos_from_depth(p-ivec2(1, 0), l1) :
//    //    -ce+worldpos_from_depth(p+ivec2(1, 0), r1);
//    //    vec3 dpdy = (db<dt) ?  ce-worldpos_from_depth(p-ivec2(0, 1), b1) :
//    //    -ce+worldpos_from_depth(p+ivec2(0, 1), t1);
//    //    return vec4(normalize(cross(dpdx, dpdy)), 1.0);
//    vec3 l1 = worldpos_from_depth(p-ivec2(1, 0), texelFetch(gDepth, p-ivec2(1, 0), 0).w);
//    vec3 r1 = worldpos_from_depth(p+ivec2(1, 0), texelFetch(gDepth, p+ivec2(1, 0), 0).w);
//    vec3 t1 = worldpos_from_depth(p+ivec2(0, 1), texelFetch(gDepth, p+ivec2(0, 1), 0).w);
//    vec3 b1 = worldpos_from_depth(p-ivec2(0, 1), texelFetch(gDepth, p-ivec2(0, 1), 0).w);
//    vec3 dpdx = r1-l1;
//    vec3 dpdy = t1-b1;
//    return vec4(normalize(cross(dpdx, dpdy)), 1.0);
//}


vec3 normal_from_depth(vec2 uv)
{
    vec2 texelSize = 1.0 / screen_resolution;

    vec2 uv_right = uv + vec2(texelSize.x, 0.0);
    vec2 uv_down = uv + vec2(0.0, texelSize.y);

    float depth_center = texture(gDepth, uv).r;
    float depth_right = texture(gDepth, uv_right).r;
    float depth_down = texture(gDepth, uv_down).r;
    depth_center = linearize_depth(depth_center);
    depth_right  = linearize_depth(depth_right);
    depth_down   = linearize_depth(depth_down);

    vec3 pos_center = worldpos_from_depth(uv, depth_center);
    vec3 pos_right  = worldpos_from_depth(uv_right, depth_right);
    vec3 pos_down   = worldpos_from_depth(uv_down, depth_down);

    vec3 tangentU = pos_right - pos_center;
    vec3 tangentV = pos_down  - pos_center;

    vec3 viewNormal = normalize(cross(tangentV, tangentU));

    mat3 invViewMat3 = mat3(inverse_view);
    vec3 worldNormal = normalize(invViewMat3 * viewNormal);
    return worldNormal;
}

vec4 create_custom_normals()
{
    vec2 uv = TexCoords;
    vec2 pos = uv * screen_resolution.xy;
    //    vec4 normal = texture(gNormal, uv);
    vec4 normal = vec4(normal_from_depth(TexCoords), 1.0);
    float normalLength = length(normal);
    float noise = EvalIGN(pos);
    vec3 stochastic_normal = get_cos_hemisphere_sample(noise, noise, normal.rgb);
    return normalize(vec4(stochastic_normal, 1.0));
}


void main()
{
    //    vec4 result = create_custom_normals();
    sNormal = create_custom_normals();

    //turn this shit stochastic
}