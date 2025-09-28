#version 460
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gORM;
layout (location = 4) out vec3 gEmission;
layout (location = 5) out uint gEntityId;

in vec4 WorldPos;
in vec3 Normal;
uniform uint entity_id;

void main()
{
    gAlbedo = vec4(1.0, 0.0, 0.0, 1.0);
    gNormal = vec4(normalize(Normal), 1.0);
    gORM = vec3(0.015, 0.54, 1.0);
    gPosition = WorldPos;
    gEmission = vec3(1.0, 1.0, 1.0);
    gEntityId = entity_id;
}