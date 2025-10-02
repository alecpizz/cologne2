//
// Created by alecpizz on 9/29/25.
//

#include "BloodSystem.h"

#include <engine/scene/Components/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void BloodSystem::on_update(Scene *scene, float dt)
    {
        auto &registry = scene->get_raw_registry();
        auto view = registry.view<BloodSplatterComponent>();
        const float blood_speed = 1.5f;
        view.each([&](entt::entity entity, BloodSplatterComponent& bsp)
        {
            bsp.time += blood_speed * dt;
            if (bsp.time >= 1.0f)
            {
                registry.destroy(entity);
            }
        });
    }
}
