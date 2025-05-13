#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

uniform mat4 model;
uniform mat4 projection;
uniform vec3 voxel_size;
uniform mat4 lightSpaceMatrix;
uniform int render_axis;

out vec4 FragPos;
out vec2 TexCoords;
out mat3 TBN;
out vec4 FragPosLightSpace;

void main()
{
    vec4 pos = model * vec4(position, 1.0f);
    FragPosLightSpace = lightSpaceMatrix * vec4(pos.xyz, 1.0);
    TexCoords = uv;
    pos = vec4((pos.xyz * voxel_size), 1.0f);
    gl_Position = pos;
    if (render_axis == 0)
    {
        gl_Position = gl_Position.zyxw;
    }
    else if(render_axis == 1)
    {
        gl_Position = gl_Position.xzyw;
    }
    FragPos = pos;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

}