#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 projection;
uniform mat4 view;

out vec3 LocalPosition;

void main()
{
    //    mat4 rotView = mat4(mat3(view));
    LocalPosition = position;
    mat4 rotView = mat4(mat3(view));
    vec4 clipPos = projection * rotView * vec4(position, 1.0);
    gl_Position = clipPos.xyww;
}