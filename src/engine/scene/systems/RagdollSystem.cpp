//
// Created by alecpizz on 9/11/25.
//

#include "RagdollSystem.h"

#include <engine/scene/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
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
        }
    }
}
