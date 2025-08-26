//
// Created by alecpizz on 8/13/25.
//

#include "AnimationSystem.h"

#include <engine/animation/AnimatorComponent.h>
#include <engine/scene/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void AnimationSystem::on_update(Scene* scene, float dt)
    {
        auto& registry = scene->get_raw_registry();
        auto animators = registry.view<AnimatorComponent, ActiveComponent>();
        for (auto entity: animators)
        {
            if (!registry.get<ActiveComponent>(entity).active)
            {
                continue;
            }
            auto &animator = registry.get<AnimatorComponent>(entity);
            animator.update(dt, registry.get<WorldTransformComponent>(entity));
        }
    }
}
