#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection_x;
uniform mat4 projection_y;
uniform mat4 projection_z;

in vec2 v_TexCoord[];

out vec2 TexCoord;
out flat int Axis;

void main()
{
    vec3 e1 = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
    vec3 e2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;

    vec3 n = normalize(cross(e1, e2));

    n = vec3(abs(n.x), abs(n.y), abs(n.z));

    float nx = n.x;
    float ny = n.y;
    float nz = n.z;
    mat4 projection;
    if(nx >= ny && nx >= nz)
    {
        Axis = 1;
        projection = projection_x;
    }
    else if(ny >= nx && ny >= nz)
    {
        Axis = 2;
        projection = projection_y;
    }
    else
    {
        Axis = 3;
        projection = projection_z;
    }

    for(int i = 0; i < gl_in.length(); i++)
    {
        TexCoord = v_TexCoord[i];
        gl_Position = projection * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}