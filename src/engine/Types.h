#pragma once
#include "../gpch.h"
#include "renderer/Material.h"
#include "renderer/Vertex.h"

namespace cologne
{
    struct MeshData
    {
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabbMax = glm::vec3(std::numeric_limits<float>::min());
        uint32_t vertex_count = 0;
        uint32_t index_count = 0;
        uint32_t parent_index = -1;
        uint32_t material_index = 0;
        glm::mat4 local_transform = glm::mat4(1.0f);
        glm::mat4 inverse_bind_transform = glm::mat4(1.0f);
    };

    struct ModelData
    {
        std::string name;
        uint32_t mesh_count;
        uint32_t material_count = 0;
        std::vector<MeshData> meshes;
        std::vector<Material> materials;
        glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabbMax = glm::vec3(std::numeric_limits<float>::min());
    };

    struct SkinnedMeshData
    {
        std::string name;
        std::vector<WeightedVertex> vertices;
        std::vector<uint32_t> indices;
        glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabbMax = glm::vec3(std::numeric_limits<float>::min());
        uint32_t vertex_count;
        uint32_t index_count;
        uint32_t base_vertex;
    };

    struct Node
    {
        std::string name;
        int parent_index;
        glm::mat4 inverse_bind_transform;
    };

    struct SkinnedModelData
    {
        std::string name;
        std::vector<SkinnedMeshData> meshes;
        std::vector<glm::mat4> boneOffsets;
        std::vector<Node> nodes;
        std::unordered_map<std::string, uint32_t> bone_map;
        uint32_t vertex_count;
        uint32_t index_count;
    };

    struct MultiDrawElementsCommand
    {
        uint32_t vertex_count;
        uint32_t instance_count;
        uint32_t first_index;
        uint32_t base_vertex;
        uint32_t base_instance;
    };
}
