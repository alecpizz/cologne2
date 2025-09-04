//
// Created by alecpizz on 9/3/25.
//

#include "RagdollSystem.h"

#include <engine/scene/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void RagdollSystem::on_update(Scene *scene, float dt)
    {
        auto& registry = scene->get_raw_registry();
        auto view = registry.view<RagdollComponent, SkeletonComponent, SkeletonPoseComponent, AnimatorComponent2, WorldTransformComponent>();

        for (auto entity : view)
        {
            auto& ragdoll = view.get<RagdollComponent>(entity);
            auto& animator = view.get<AnimatorComponent2>(entity);
            auto& skeleton = view.get<SkeletonComponent>(entity);
            auto& pose = view.get<SkeletonPoseComponent>(entity);
            auto& transform = view.get<WorldTransformComponent>(entity).transform;

            //todo: remove ts
            if (animator.requested_state == AnimatorComponent2::StateRequest::TO_RAGDOLL)
            {
                if (ragdoll.current_state != RagdollComponent::State::ACTIVE)
                {
                    Physics::make_ragdoll_active(ragdoll.ragdoll_id);
                }
            }
            else if (animator.requested_state == AnimatorComponent2::StateRequest::TO_KINEMATIC)
            {
                if (ragdoll.current_state != RagdollComponent::State::KINEMATIC)
                {
                    Physics::make_ragdoll_kinematic(ragdoll.ragdoll_id);
                }
            }
            animator.requested_state = AnimatorComponent2::StateRequest::NONE;

            if (ragdoll.current_state == RagdollComponent::State::ACTIVE)
            {
                std::unordered_map<std::string, glm::mat4> ragdoll_transforms(ragdoll.bone_to_rigidbody_map.size());
                for (auto &pair: ragdoll.bone_to_rigidbody_map)
                {
                    int bone_idx = skeleton.skeleton.find_bone_index(pair.first);
                    ragdoll_transforms[pair.first] = transform * pose.global_transforms[bone_idx];
                }
                Physics::sync_ragdoll(ragdoll.ragdoll_id, ragdoll_transforms);
            }
            else
            {
                glm::mat4 inverse_entity_transform = glm::inverse(transform);
                for (int i = 0; i < skeleton.skeleton.get_bone_count(); i++)
                {
                    const auto &bone = skeleton.skeleton.get_bones()[i];
                    std::string node_name = bone.name;
                    glm::mat4 node_transform = bone.local_bind_transform;
                    unsigned int parent_idx = bone.parent_idx;
                    glm::mat4 parent_transform = (parent_idx == -1) ? glm::mat4(1.0f) : pose.global_transforms[parent_idx];
                    glm::mat4 global_transform = parent_transform * node_transform;

                    if (ragdoll.bone_to_rigidbody_map.contains(node_name))
                    {
                        global_transform = inverse_entity_transform * Physics::get_rigidbody_transform(
                                               ragdoll.bone_to_rigidbody_map[node_name]);
                    }

                    pose.global_transforms[i] = global_transform;
                }
                const auto& bones = skeleton.skeleton.get_bones();
                for (size_t i = 0; i < bones.size(); i++)
                {
                    glm::vec3 scale = glm::vec3(pose.global_transforms[i][0][0], pose.global_transforms[i][1][1], pose.global_transforms[i][2][2]);
                    if (scale.x > 1.1f)
                    {
                        LOG_INFO("Big bone %s", bones[i].name.c_str());
                    }
                    pose.skinning_matrices[i] = pose.global_transforms[i] * bones[i].inverse_bind_pose;
                }
            }
        }
    }
}
