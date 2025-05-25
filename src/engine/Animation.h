#pragma once
#include "gpch.h"
#include "assimp/matrix4x4.h"
#include "renderer/Vertex.h"

namespace cologne::Animation
{
    struct Bone
    {
        uint32_t id = 0;
        std::string name = "";
        glm::mat4 offset = glm::mat4(1.0f);
    };

    struct BoneTransformTrack
    {
        std::vector<float> position_times = {};
        std::vector<float> rotation_times = {};
        std::vector<float> scale_times = {};

        std::vector<glm::vec3> positions = {};
        std::vector<glm::vec3> scales = {};
        std::vector<glm::quat> rotations = {};
    };

    struct Animation
    {
        float duration = 0.0f;
        float ticks_per_second = 1.0f;
        std::unordered_map<std::string, BoneTransformTrack> bone_transforms = {};
    };

    struct SkinnedMeshData
    {
        std::vector<WeightedVertex> vertices = {};
        std::vector<uint32_t> indices = {};
    };

    struct SkinnedModelData
    {
        glm::mat4 global_inverse_transform;
        std::vector<SkinnedMeshData> meshes;
        std::vector<Bone> bones;
    };

    SkinnedModelData load_model(const std::string &path);

    glm::mat4 assimp_to_glm_matrix(const aiMatrix4x4 &from);
}
