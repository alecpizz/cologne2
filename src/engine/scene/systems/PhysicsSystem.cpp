//
// Created by alecpizz on 8/13/25.
//

#include "PhysicsSystem.h"
#include <engine/scene/Components.h>
#include <engine/scene/Entity.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void PhysicsSystem::on_update(Scene* scene, float dt)
    {
        auto &registry = scene->get_raw_registry();
        //temp?
        auto view = registry.view<StaticColliderComponent, WorldTransformComponent>();
        for (auto entity: view)
        {
            Entity e = {entity, scene};
            if (e.get_component<ActiveComponent>().active)
            {
                if (!e.get_component<StaticColliderComponent>().body_enabled)
                {
                    Physics::enable_body(e.get_component<StaticColliderComponent>().body_id);
                    e.get_component<StaticColliderComponent>().body_enabled = true;
                }
                Physics::sync_transform(e);
            }
            else
            {
                if (e.get_component<StaticColliderComponent>().body_enabled)
                {
                    Physics::disable_body(e.get_component<StaticColliderComponent>().body_id);
                    e.get_component<StaticColliderComponent>().body_enabled = false;
                }
            }
        }

        for (auto e : registry.view<RigidbodyComponent, TransformComponent>())
        {
            auto& tr = registry.get<TransformComponent>(e);
            auto& rb = registry.get<RigidbodyComponent>(e);
            if (rb.body_id == 0)
            {
                continue;
            }
            auto mat = Physics::get_rigidbody_transform(rb.body_id);
            tr = TransformComponent(mat);
        }
    }
}
