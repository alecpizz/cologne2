#pragma once

namespace cologne
{
    #pragma pack (push, 1)
    struct ModelCacheHeader
    {
        uint32_t mesh_count;
        uint32_t material_count;
        uint64_t time_saved;
        glm::vec3 aabb_min;
        glm::vec3 aabb_max;
        char name[256];
    };

    struct SkinnedModelCacheHeader
    {
        uint32_t mesh_count;
        uint32_t material_count;
        uint64_t time_saved;
        glm::vec3 aabb_min;
        glm::vec3 aabb_max;
        char name[256];
        uint32_t bone_count;
        uint32_t animation_count;
    };

    struct MeshCacheHeader
    {
        char name[512];
        uint32_t vertex_count;
        uint32_t index_count;
        glm::vec3 aabb_min;
        glm::vec3 aabb_max;
        uint32_t material_index;
        glm::mat4 inverse_bind_pose;
    };

    struct BoneMappingHeader
    {
        char name[512];
        int index;
    };

    struct BoneHeader
    {
        char name[512];
        glm::mat4 inverse_bind_pose;
        glm::mat4 local_bind_transform;
        int parent_idx = -1;
    };

    struct MaterialCacheHeader
    {
        char albedo_path[512];
        char normal_path[512];
        char metallic_path[512];
        char roughness_path[512];
        char ao_path[512];
        char emission_path[512];
        float roughness_override = 1.0f;
        float metallic_override = 1.0f;
    };

    struct AnimationClipCacheHeader
    {
        uint32_t channel_count;
        char name[512];
        float duration = 0.0f;
        int ticks_per_second = 0;
    };

    struct KeyframeCacheHeader
    {
        char name[512];
        uint32_t position_count;
        uint32_t scale_count;
        uint32_t rotation_count;
    };
#pragma pack(pop)
}
