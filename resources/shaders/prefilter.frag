#version 460 core
out vec4 FragColor;
in vec3 LocalPosition;

layout (binding = 0) uniform samplerCube environment_map;
uniform float roughness;

float radical_inverse_vdc(uint bits);
vec2 hammersley(uint i, uint n);
vec3 importance_sample_ggx(vec2 xi, vec3 n, float roughness);
float distribution_ggx(vec3 n, vec3 h, float roughness);
const float PI = 3.14159265359;

void main()
{
    vec3 N = normalize(LocalPosition);
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float total_weight = 0.0;
    vec3 prefiltered_color = vec3(0.0);

    for (uint i = 0u; i < SAMPLE_COUNT; i++)
    {
        vec2 xi = hammersley(i, SAMPLE_COUNT);
        vec3 h = importance_sample_ggx(xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, h) * h - V);
        float NDotL = max(dot(N, L), 0.0);
        if (NDotL > 0.0)
        {
            float D = distribution_ggx(N, h, roughness);
            float NdotH = max(dot(N, h), 0.0);
            float HdotV = max(dot(h, V), 0.0);
            float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;
            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
            prefiltered_color += textureLod(environment_map, L, mipLevel).rgb * NDotL;
            total_weight += NDotL;
        }
    }
    prefiltered_color = prefiltered_color / total_weight;
    FragColor = vec4(prefiltered_color, 1.0);
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

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + n * H.z;
    return normalize(sampleVec);
}

float distribution_ggx(vec3 n, vec3 h, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(n, h), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
