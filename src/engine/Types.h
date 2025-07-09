#pragma once
#include "animation/AnimationClip.h"
#include "animation/Skeleton.h"
#include "renderer/types/Material.h"
#include "renderer/types/Vertex.h"

namespace cologne
{
    class Skeleton;
}

namespace cologne
{
    struct MeshData
    {
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        glm::vec3 aabb_min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabb_max = glm::vec3(std::numeric_limits<float>::min());
        uint32_t material_index;
        glm::mat4 inverse_bind_pose = glm::mat4(1.0f);
        int32_t base_vertex = 0;
        uint32_t first_index = 0;
    };

    struct SkinnedMeshData
    {
        std::string name;
        std::vector<WeightedVertex> vertices;
        std::vector<uint32_t> indices;
        glm::vec3 aabb_min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabb_max = glm::vec3(std::numeric_limits<float>::min());
        uint32_t material_index;
        int32_t base_vertex = 0;
        uint32_t first_index = 0;
    };

    struct ModelData
    {
        std::string name;
        std::vector<MeshData> meshes;
        std::vector<Material> materials;
        glm::vec3 aabb_min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabb_max = glm::vec3(std::numeric_limits<float>::min());
    };

    struct BoneInfo;
    class Animation;

    class Skeleton;
    struct SkinnedModelData
    {
        std::string name;
        std::vector<SkinnedMeshData> meshes;
        std::vector<Material> materials;
        glm::vec3 aabb_min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabb_max = glm::vec3(std::numeric_limits<float>::min());
        std::vector<AnimationClip> animations;
        uint32_t material_index;
        Skeleton skeleton;
    };


    struct MultiDrawElementsCommand
    {
        uint32_t index_count;
        uint32_t instance_count;
        uint32_t first_index;
        uint32_t base_vertex;
        uint32_t base_instance;
    };

    struct PlayerCreateInfo
    {
        float height_standing = 1.35f;
        float radius_standing = 0.3f;
        float height_crouching = 0.8f;
        float radius_crouching = 0.3f;
        float inner_friction = 0.9f;
        float character_speed = 3.5f;
        float jump_speed = 4.0f;
        glm::vec3 position = glm::vec3(0.0f);
    };

    struct PlayerMovementCommand
    {
        glm::vec3 movement;
        glm::quat rotation;
        glm::vec3 up;
    };
}
