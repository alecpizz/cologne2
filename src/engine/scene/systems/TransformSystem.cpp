//
// Created by alecpizz on 8/13/25.
//

#include "TransformSystem.h"

#include <engine/scene/Components.h>
#include <engine/scene/Entity.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void TransformSystem::on_update(Scene* scene, float dt)
    {
        auto& registry = scene->get_raw_registry();

        for (auto entity: registry.view<WorldTransformComponent, TransformComponent>())
        {
            auto &tr = registry.get<TransformComponent>(entity);
            auto &wd = registry.get<WorldTransformComponent>(entity);
            wd.transform = tr.get_mat4();
        }

        auto parent_view = registry.view<ParentComponent>();
        for (auto entity: parent_view)
        {
            update_children(scene, entity, registry);
        }
    }

    void TransformSystem::update_children(Scene* scene, entt::entity parent, entt::registry &registry)
    {
        auto &parent_wld = registry.get<WorldTransformComponent>(parent);
        auto &parent_comp = registry.get<ParentComponent>(parent);

        for (auto child_id: parent_comp.children)
        {
            Entity child = scene->get_entity_by_uuid(child_id);
            if (registry.valid(child))
            {
                auto &child_transform = registry.get<TransformComponent>(child);
                auto &child_world_transform = registry.get<WorldTransformComponent>(child);
                child_world_transform.transform = parent_wld.transform * child_transform.get_mat4();
                if (registry.any_of<ParentComponent>(child))
                {
                    update_children(scene, child, registry);
                }
            }
        }
    }
}
