#version 460 core
layout (location = 0) in vec3 position;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform int voxel_size;

out vec3 FragPos;
out vec3 TexCoord;

void main()
{
    int index = gl_InstanceID;
    int gridWidth = voxel_size;
    int gridHeight = voxel_size;
    int layerSize = gridHeight * gridWidth;

    int z = index / layerSize;
    int indexInLayer = index % layerSize;
    int y = indexInLayer / gridWidth;
    int x = indexInLayer % gridWidth;

    ivec3 voxelCoord = ivec3(x, y, z);
    float cellSize = 1.0f;
    vec3 instanceOffset = vec3(voxelCoord) * cellSize;
    FragPos = instanceOffset + position * cellSize;
    TexCoord = (vec3(voxelCoord) + 0.5) / vec3(float(voxel_size));

    gl_Position = projection * view * vec4(FragPos, 1.0);
}