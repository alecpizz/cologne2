#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

in vec2 UV[];
in vec3 Norm[];
in vec3 Pos[];

out vec2 TexCoords;
out vec3 Position;
out vec3 Normal;
uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle vertex
        {
            FragPos = gl_in[i].gl_Position;
            TexCoords = UV[i];
            Position = Pos[i];
            Normal = Norm[i];
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}