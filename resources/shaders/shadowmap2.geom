#version 460 core

layout(triangles, invocations = 5) in;
layout (triangle_strip, max_vertices = 3) out;

layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

in vec2 v_TexCoords[];
in vec3 v_Normal[];
in vec4 v_Position[];
out vec2 g_TexCoords;
out vec3 g_Normal;
out vec4 g_Position;

void main()
{
    for(int i = 0; i < 3; i++)
    {
        gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        g_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        g_Normal = v_Normal[i];
        g_TexCoords = v_TexCoords[i];
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}