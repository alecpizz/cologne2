//
// Created by alecpizz on 8/13/25.
//

#include "PhysicsSystem.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/scene/Entity.h>
#include <engine/scene/Scene.h>
#include <engine/scene/components/ActiveComponent.h>
#include <engine/scene/components/RigidbodyComponent.h>
#include <engine/scene/components/ConvexMeshColliderComponent.h>
#include <engine/scene/components/StaticColliderComponent.h>
#include <engine/scene/components/TransformComponent.h>
#include <engine/scene/components/WorldTransformComponent.h>

namespace cologne
{
    void PhysicsSystem::on_scene_start(Scene *scene)
    {
        auto &registry = scene->get_raw_registry();
        auto view = registry.view<TransformComponent, StaticColliderComponent>();
        for (auto entity: view)
        {
            Entity e = {entity, scene};
            auto &transform = registry.get<TransformComponent>(entity);
            auto &collider = registry.get<StaticColliderComponent>(entity);
            auto mesh = collider.mesh.get();
            if (!mesh)
            {
                continue;
            }
            uint32_t body_id = Physics::create_static_mesh_collider(
                e, transform, *mesh);
            collider.body_id = body_id;
        }

        for (auto entity: registry.view<RigidbodyComponent, ConvexMeshColliderComponent>())
        {
            Entity e = {entity, scene};
            auto &rb = registry.get<RigidbodyComponent>(entity);
            rb.body_id = Physics::create_rigidbody(e);
        }
    }

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
