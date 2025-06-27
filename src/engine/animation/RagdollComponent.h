//
// Created by alecpizz on 6/26/25.
//
#pragma once
#include <engine/physics/RagdollBone.h>


namespace cologne
{
    class Animation;
    class SkinnedModel;
}

namespace cologne
{
    struct BoneInfo;
    struct Node;

    class RagdollComponent
    {
    public:
        RagdollComponent(SkinnedModel* model, Animation* animation);

        ~RagdollComponent();

        void activate_ragdoll();

        void deactivate_ragdoll();

        void update_kinematic(const std::vector<glm::mat4> &final_bones, const glm::mat4 &model_world);

        void update_skinned_mesh_bones(std::vector<glm::mat4> &bone_output,
                                       const std::unordered_map<std::string, BoneInfo> &bone_info_map);

        bool is_ragdoll() const { return _is_ragdoll; }

        float scale = 0.001f;

    private:
        bool _is_ragdoll = false;
        bool should_create_body(const char* name);
        std::vector<RagdollBone> _ragdollBones;
        Node* _root =  nullptr;
    };
}
