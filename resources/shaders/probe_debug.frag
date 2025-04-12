#version 460 core

layout (location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler3D probe_lighting;
in vec3 Color;

in flat int ProbeIndex;
in vec3 ProbeWorldPos;
in vec3 FragPos;

layout(binding = 2, std430) buffer shCoefficients
{
    vec3 L1SH_0[3750 * 2];
    vec3 L1SH_1[3750 * 2];
    vec3 L1SH_2[3750 * 2];
    vec3 L1SH_3[3750 * 2];
};
#define PI 3.1415926535897932384626433832795
#define myT vec3
#define myL 1
#define SphericalHarmonicsTL(T, L) T[(L + 1)*(L + 1)]
#define SphericalHarmonics SphericalHarmonicsTL(myT, myL)
#define shSize(L) ((L + 1)*(L + 1))

myT shDot(in SphericalHarmonics shA, in SphericalHarmonics shB)
{
    myT result = myT(0.0);
    for (int i = 0; i < shSize(myL); ++i)
    {
        result += shA[i] * shB[i];
    }
    return result;
}


const float sqrtPI = 1.7724538509055160272981674833411;
SphericalHarmonics shEvaluate(vec3 p)
{
    // From Peter-Pike Sloan's Stupid SH Tricks
    // http://www.ppsloan.org/publications/StupidSH36.pdf
    // https://github.com/dariomanesku/cmft/blob/master/src/cmft/cubemapfilter.cpp#L130

    SphericalHarmonics result;

    float x = -p.x;
    float y = -p.y;
    float z = p.z;

    float x2 = x*x;
    float y2 = y*y;
    float z2 = z*z;

    float z3 = z2*z;

    float x4 = x2*x2;
    float y4 = y2*y2;
    float z4 = z2*z2;

    int i = 0;

    result[i++] =  myT(1.0f/(2.0f*sqrtPI));

    #if (myL >= 1)
    result[i++] = myT(-sqrt(3.0f/(4.0f*PI))*y);
    result[i++] = myT(sqrt(3.0f/(4.0f*PI))*z);
    result[i++] = myT(-sqrt(3.0f/(4.0f*PI))*x);
    #endif

    #if (myL >= 2)
    result[i++] = myT(sqrt(15.0f/(4.0f*PI))*y*x);
    result[i++] = myT(-sqrt(15.0f/(4.0f*PI))*y*z);
    result[i++] = myT(sqrt(5.0f/(16.0f*PI))*(3.0f*z2-1.0f));
    result[i++] = myT(-sqrt(15.0f/(4.0f*PI))*x*z);
    result[i++] = myT(sqrt(15.0f/(16.0f*PI))*(x2-y2));
    #endif

    #if (myL >= 3)
    result[i++] = myT(-sqrt(70.0f/(64.0f*PI))*y*(3.0f*x2-y2));
    result[i++] = myT(sqrt(105.0f/ (4.0f*PI))*y*x*z);
    result[i++] = myT(-sqrt(21.0f/(16.0f*PI))*y*(-1.0f+5.0f*z2));
    result[i++] = myT(sqrt(7.0f/(16.0f*PI))*(5.0f*z3-3.0f*z));
    result[i++] = myT(-sqrt(42.0f/(64.0f*PI))*x*(-1.0f+5.0f*z2));
    result[i++] = myT(sqrt(105.0f/(16.0f*PI))*(x2-y2)*z);
    result[i++] = myT(-sqrt(70.0f/(64.0f*PI))*x*(x2-3.0f*y2));
    #endif

    #if (myL >= 4)
    result[i++] = myT(3.0f*sqrt(35.0f/(16.0f*PI))*x*y*(x2-y2));
    result[i++] = myT(-3.0f*sqrt(70.0f/(64.0f*PI))*y*z*(3.0f*x2-y2));
    result[i++] = myT(3.0f*sqrt(5.0f/(16.0f*PI))*y*x*(-1.0f+7.0f*z2));
    result[i++] = myT(-3.0f*sqrt(10.0f/(64.0f*PI))*y*z*(-3.0f+7.0f*z2));
    result[i++] = myT((105.0f*z4-90.0f*z2+9.0f)/(16.0f*sqrtPI));
    result[i++] = myT(-3.0f*sqrt(10.0f/(64.0f*PI))*x*z*(-3.0f+7.0f*z2));
    result[i++] = myT(3.0f*sqrt(5.0f/(64.0f*PI))*(x2-y2)*(-1.0f+7.0f*z2));
    result[i++] = myT(-3.0f*sqrt(70.0f/(64.0f*PI))*x*z*(x2-3.0f*y2));
    result[i++] = myT(3.0f*sqrt(35.0f/(4.0f*(64.0f*PI)))*(x4-6.0f*y2*x2+y4));
    #endif

    return result;
}

vec3 GetRadianceFromSH(SphericalHarmonics shRadiance, vec3 direction)
{
    SphericalHarmonics shDirection = shEvaluate(direction);

    vec3 sampleSh = max(vec3(0.0), shDot(shRadiance, shDirection));
    return sampleSh;
}

void main()
{
    vec3 dir = ProbeWorldPos - FragPos;

    SphericalHarmonics radiance;
    radiance[0] = L1SH_0[ProbeIndex];
    radiance[1] = L1SH_1[ProbeIndex];
    radiance[2] = L1SH_2[ProbeIndex];
    radiance[3] = L1SH_3[ProbeIndex];

    vec3 col = GetRadianceFromSH(radiance, dir);

    FragColor.rgb = col;
    FragColor.a = 1.0f;
}