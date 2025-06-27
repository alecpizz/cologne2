//
// Created by alecpizz on 6/26/25.
//

#include "RagdollComponent.h"

#include <engine/physics/Physics.h>
#include <engine/util/Util.h>

#include "Animation.h"
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Ragdoll/Ragdoll.h"

namespace cologne
{
    static std::unordered_map<std::string, std::string> phys_to_mixamo_map = {
        {"LowerBody", "mixamorig1:Hips"},
        {"MidBody", "mixamorig1:Spine"},
        {"UpperBody", "mixamorig1:Spine1"},
        {"Head", "mixamorig1:Head"},
        {"UpperArmL", "mixamorig1:LeftArm"},
        {"LowerArmL", "mixamorig1:LeftForearm"},
        {"UpperArmR", "mixamorig1:RightArm"},
        {"LowerArmR", "mixamorig1:RightForearm"},
        {"UpperLegL", "mixamorig1:LeftUpLeg"},
        {"LowerLegL", "mixamorig1:LeftLeg"},
        {"UpperLegR", "mixamorig1:RightUpLeg"},
        {"LowerLegR", "mixamorig1:RightLeg"},
    };

    RagdollComponent::RagdollComponent(SkinnedModel *model, Animation *animation)
    {
        JPH::Ragdoll *ragdoll = Physics::create_ragdoll(glm::vec3(0.0f));
        auto *settings = ragdoll->GetRagdollSettings();
        for (size_t i = 0; i < ragdoll->GetBodyIDs().size(); i++)
        {
            auto body_id = ragdoll->GetBodyID(i);
            auto skeleton_joint = settings->mSkeleton->GetJoint(i);
            std::string bone_name = phys_to_mixamo_map[skeleton_joint.mName.c_str()];
            _ragdollBones.push_back(RagdollBone(body_id.GetIndexAndSequenceNumber(), bone_name,
                                                model->_bone_info_map[bone_name].offset));
        }
        _root = &animation->get_root();
    }

    RagdollComponent::~RagdollComponent()
    {
    }

    void RagdollComponent::activate_ragdoll()
    {
    }

    void RagdollComponent::deactivate_ragdoll()
    {
    }

    void RagdollComponent::update_kinematic(const std::vector<glm::mat4> &final_bones, const glm::mat4 &model_world)
    {
    }

    void update_all_bone_nodes_recursively(Node &node, const glm::mat4 &parent,
                                           const std::unordered_map<std::string, glm::mat4> &phys_bones,
                                           const std::unordered_map<std::string, BoneInfo> &bone_info_map,
                                           std::vector<glm::mat4> &out_bone_matrices)
    {
        glm::mat4 local_transform;
        if (phys_bones.contains(node.name))
        {
            local_transform = glm::inverse(parent) * phys_bones.at(node.name);
        }
        else
        {
            local_transform = node.transform;
        }

        glm::mat4 global = parent * local_transform;
        if (bone_info_map.contains(node.name))
        {
            int idx = bone_info_map.at(node.name).id;
            glm::mat4 offset = bone_info_map.at(node.name).offset;
            out_bone_matrices[idx] = global * offset;
        }

        for (auto &child: node.children)
        {
            update_all_bone_nodes_recursively(child, global, phys_bones, bone_info_map, out_bone_matrices);
        }
    }

    void RagdollComponent::update_skinned_mesh_bones(std::vector<glm::mat4> &bone_output,
                                                     const std::unordered_map<std::string, BoneInfo> &bone_info_map)
    {
        std::unordered_map<std::string, glm::mat4> phys_bone_transforms;
        for (const auto &ragdollBone: _ragdollBones)
        {
            JPH::Vec3 pos;
            JPH::Quat rot;
            JPH::BodyInterface &body_interface = Physics::get_body_interface();
            body_interface.GetPositionAndRotation(JPH::BodyID(ragdollBone.body_id), pos, rot);

            glm::mat4 body_world = glm::translate(glm::mat4(1.0f), Util::jph_vec3_to_glm_vec3(pos)) * glm::toMat4(
                                       Util::jph_quat_to_glm_quat(rot)) * glm::scale(glm::mat4(1.0f), glm::vec3(scale));

            glm::mat4 bone_world_transform = body_world * ragdollBone.initial_body_to_bone_transform;
            phys_bone_transforms[ragdollBone.bone_name] = bone_world_transform;
        }

        update_all_bone_nodes_recursively(*_root, glm::mat4(1.0f), phys_bone_transforms, bone_info_map, bone_output);
    }


    bool RagdollComponent::should_create_body(const char *name)
    {
    }
}
