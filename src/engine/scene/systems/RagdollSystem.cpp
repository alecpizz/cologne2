//
// Created by alecpizz on 9/11/25.
//

#include "RagdollSystem.h"
#include <engine/asset_manager/AssetManager.h>
#include <engine/scene/Components.h>
#include <engine/scene/Entity.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void RagdollSystem::on_scene_start(Scene *scene)
    {
        auto& registry = scene->get_raw_registry();
        for (const auto entity : registry.view<RagdollComponent, SkinnedModelComponent>())
        {
            auto& rd = registry.get<RagdollComponent>(entity);
            auto& sm = registry.get<SkinnedModelComponent>(entity);
            auto model = AssetManager::get_skinned_model_by_name(sm.model_name);
            if (!model)
            {
                continue;
            }
            if (rd.id != UINT32_MAX)
            {
                continue;
            }
            //make it make it don't fake it
            SkeletonPose pose(sm.skeleton);
            for (size_t i = 0; i < sm.skeleton.get_bone_count(); i++)
            {
                pose.local_transforms[i] = sm.skeleton.get_bones()[i].local_bind_transform;
            }
            pose.update_skinning_matrices(sm.skeleton);
            rd.id = Physics::create_ragdoll({entity, scene},
                rd.bone_to_ragdoll_map, sm.skeleton, pose.global_transforms);
            if (rd.current_state == RagdollComponent::State::KINEMATIC)
            {
                Physics::make_ragdoll_kinematic(rd.id);
            }
        }
    }

    void RagdollSystem::on_update(Scene *scene, float dt)
    {
        auto& registry = scene->get_raw_registry();
        auto ragdolls = registry.view<SkinnedModelComponent, RagdollComponent, ActiveComponent, WorldTransformComponent>();
        for (auto entity : ragdolls)
        {
            if (!registry.get<ActiveComponent>(entity).active)
            {
                continue;
            }
            auto& transform = registry.get<WorldTransformComponent>(entity).transform;
            auto& sm = registry.get<SkinnedModelComponent>(entity);
            auto& rd = registry.get<RagdollComponent>(entity);
            if (rd.id == UINT32_MAX)
            {
                continue;
            }

            if (rd.current_state == RagdollComponent::State::KINEMATIC)
            {
                std::unordered_map<std::string, glm::mat4> ragdoll_transforms(rd.bone_to_ragdoll_map.size());
                for (auto &pair: rd.bone_to_ragdoll_map)
                {
                    int bone_idx = sm.skeleton.find_bone_index(pair.first);
                    ragdoll_transforms[pair.first] = transform * sm.skeleton_pose.global_transforms[bone_idx];
                }
                Physics::sync_ragdoll(rd.id, ragdoll_transforms);
            }
            else if (rd.current_state == RagdollComponent::State::ACTIVE)
            {
                glm::mat4 inverse_entity_transform = glm::inverse(transform);

                for (int i = 0; i < sm.skeleton.get_bone_count(); i++)
                {
                    const auto &bone = sm.skeleton.get_bones()[i];
                    std::string node_name = bone.name;
                    glm::mat4 node_transform = sm.skeleton_pose.local_transforms[i];
                    unsigned int parent_idx = bone.parent_idx;
                    glm::mat4 parent_transform = (parent_idx == -1) ? glm::mat4(1.0f) : sm.skeleton_pose.global_transforms[parent_idx];
                    glm::mat4 animated_global_transform = parent_transform * node_transform;
                    glm::mat4 final_transform;

                    if (rd.bone_to_ragdoll_map.contains(node_name))
                    {
                        glm::vec3 animated_scale;
                        glm::vec3 dummy_pos;
                        glm::quat dummy_rot;
                        Util::decompose_mat4(animated_global_transform, dummy_pos, dummy_rot, animated_scale);
                        glm::mat4 rigidbody_world_transform = Physics::get_rigidbody_transform(rd.bone_to_ragdoll_map[node_name]);
                        glm::mat4 scaled_rigidbody_world_transform = rigidbody_world_transform * glm::scale(glm::mat4(1.0f), animated_scale);
                        final_transform = inverse_entity_transform * scaled_rigidbody_world_transform;
                    }
                    else
                    {
                        final_transform = animated_global_transform;
                    }

                    sm.skeleton_pose.global_transforms[i] = final_transform;
                }
                sm.skeleton_pose.update_skinning_matrices_no_rebuild(sm.skeleton);
            }
        }
    }


}
