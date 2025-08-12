//
// Created by alecpizz on 7/19/25.
//
#include "Components.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/renderer/Renderer.h>

namespace cologne
{
    MeshComponent::MeshComponent(const std::string &name)
    {
        mesh_name = name;
    }

    MeshComponent::MeshComponent(int idx)
    {
        auto mesh_by_index = AssetManager::get_mesh_by_index(idx);
        if (!mesh_by_index)
        {
            return;
        }
        mesh_name = mesh_by_index->get_name();
    }

    void StaticColliderComponent::on_destroy(entt::registry &registry, const entt::entity entt)
    {
        auto col = registry.get<StaticColliderComponent>(entt);
        Physics::destroy_body(col.body_id);
    }

    void LightComponent::on_construct(entt::registry &registry, const entt::entity entt)
    {
        auto& light_comp = registry.get<LightComponent>(entt);
        auto& transform = registry.get_or_emplace<WorldTransformComponent>(entt);

        LightHandle handle = Engine::get_renderer()->create_light(Light(light_comp, TransformComponent(transform)));
        registry.emplace_or_replace<LightHandleComponent>(entt, handle);
    }

    void LightComponent::on_destroy(entt::registry &registry, const entt::entity entt)
    {
        auto& handle_comp = registry.get_or_emplace<LightHandleComponent>(entt);
        if (handle_comp.light_handle.is_valid())
        {
            Engine::get_renderer()->destroy_light(handle_comp.light_handle);
        }
    }
}
