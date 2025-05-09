#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 g_world_pos[];
in vec4 FragPosLightSpace[];
//in vec3 g_normal[];
in vec2 g_tex_coords[];
in mat3 TBN[];
//out vec3 f_normal;
out vec2 f_tex_coords;
out vec3 f_voxel_pos;// world coordinates scaled to clip space (-1...1)
out vec4 f_shadow_coords;
out mat3 f_TBN;
out vec4 f_frag_pos_light_space;

void main()
{
    mat3 swizzle_mat;

    const vec3 edge1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    const vec3 edge2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    const vec3 face_normal = abs(cross(edge1, edge2));

    if (face_normal.x >= face_normal.y && face_normal.x >= face_normal.z)
    {
        swizzle_mat = mat3(
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 1.0, 0.0),
        vec3(1.0, 0.0, 0.0));
    } else if (face_normal.y >= face_normal.z)
    {
        swizzle_mat = mat3(
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 1.0, 0.0));
    } else
    {
        swizzle_mat = mat3(
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0));
    }

    for (int i=0; i < 3; i++)
    {
        gl_Position = vec4(gl_in[i].gl_Position.xyz * swizzle_mat, 1.0f);
        f_voxel_pos = gl_in[i].gl_Position.xyz;
        f_frag_pos_light_space = FragPosLightSpace[i];
        f_tex_coords = g_tex_coords[i];
        f_TBN = TBN[i];
        EmitVertex();
    }

    EndPrimitive();
}