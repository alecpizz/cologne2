#pragma once
#include "renderer/types/Material.h"
#include "renderer/types/Vertex.h"

namespace cologne
{
    struct BoneInfo;

    struct MeshData
    {
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        glm::vec3 aabb_min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabb_max = glm::vec3(std::numeric_limits<float>::min());
        uint32_t material_index;
    };

    struct SkinnedMeshData
    {
        std::string name;
        std::vector<WeightedVertex> vertices;
        std::vector<uint32_t> indices;
        glm::vec3 aabb_min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabb_max = glm::vec3(std::numeric_limits<float>::min());
        uint32_t material_index;
    };

    struct ModelData
    {
        std::string name;
        std::vector<MeshData> meshes;
        std::vector<Material> materials;
        glm::vec3 aabb_min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabb_max = glm::vec3(std::numeric_limits<float>::min());
    };

    struct SkinnedModelData
    {
        std::string name;
        std::vector<SkinnedMeshData> meshes;
        std::vector<Material> materials;
        glm::vec3 aabb_min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabb_max = glm::vec3(std::numeric_limits<float>::min());
        std::unordered_map<std::string, BoneInfo> bone_map;
        int bone_count = 0;
        uint32_t material_index;
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
