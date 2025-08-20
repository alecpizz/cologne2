//
// Created by alecpizz on 8/13/25.
//

#include "AnimationSystem.h"

#include <engine/animation/AnimatorComponent.h>
#include <engine/core/Engine.h>
#include <engine/scene/Components.h>

namespace cologne
{
    void AnimationSystem::on_update(float dt)
    {
        auto& registry = _scene->get_raw_registry();
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
