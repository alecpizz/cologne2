#version 460 core
out vec2 FragColor;
in vec2 TexCoords;

const float PI = 3.14159265359;

float radical_inverse_vdc(uint bits);
vec2 hammersley(uint i, uint n);
vec3 importance_sample_ggx(vec2 xi, vec3 n, float roughness);
float geometry_schlick_ggx(float ndotv, float roughness);
float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness);
vec2 integrate_brdf(float ndotv, float rougness);

void main()
{
    vec2 integrated_brdf = integrate_brdf(TexCoords.x, TexCoords.y);
    FragColor = integrated_brdf;
}

float radical_inverse_vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint n)
{
    return vec2(float(i) / float(n), radical_inverse_vdc(i));
}

vec3 importance_sample_ggx(vec2 xi, vec3 n, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates - halfway vector
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space H vector to world-space sample vector
    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + n * H.z;
    return normalize(sampleVec);
}
float geometry_schlick_ggx(float ndotv, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom = ndotv;
    float denom = ndotv * (1.0 - k) + k;

    return nom / denom;
}


float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness)
{
    float NdotV = max(dot(n, v), 0.0);
    float NdotL = max(dot(n, l), 0.0);
    float ggx2 = geometry_schlick_ggx(NdotV, roughness);
    float ggx1 = geometry_schlick_ggx(NdotL, roughness);

    return ggx1 * ggx2;
}

vec2 integrate_brdf(float ndotv, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - ndotv*ndotv);
    V.y = 0.0;
    V.z = ndotv;

    float A = 0.0;
    float B = 0.0;

    vec3 N = vec3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the
        // preferred alignment direction (importance sampling).
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H = importance_sample_ggx(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            float G = geometry_smith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * ndotv);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}