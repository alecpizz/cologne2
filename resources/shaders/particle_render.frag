#version 460 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gORM;
layout (location = 4) out vec3 gEmission;

in vec3 FragPos;

uniform mat4 view;

void main()
{
    vec2 tex_coords = gl_PointCoord * 2.0 - 1.0;

    float mag = dot(tex_coords, tex_coords);
    if(mag > 1.0)
    {
        discard;
    }

    float z = sqrt(1.0 - mag);
    vec3 normal_view = vec3(tex_coords.x, -tex_coords.y, z);

    mat3 inv_view = transpose(mat3(view));
    vec3 normal_world = normalize(inv_view * normal_view);

    gPosition = vec4(FragPos, 1.0);
    gNormal = vec4(normal_world, 1.0);
    gAlbedo = vec4(0.8, 0.8, 0.8, 1.0);
    gORM = vec3(0.5, 0.5, 0.02);
    gEmission = vec3(0.0);
}