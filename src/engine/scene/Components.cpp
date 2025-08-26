//
// Created by alecpizz on 7/19/25.
//
#include "Components.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/renderer/Renderer.h>

namespace cologne
{
    void RigidbodyComponent::on_construct(entt::registry &registry, const entt::entity entt)
    {
    }

    void RigidbodyComponent::on_destroy(entt::registry &registry, const entt::entity entt)
    {
        auto col = registry.get<RigidbodyComponent>(entt);
        Physics::destroy_body(col.body_id);
    }

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

    void PlayerComponent::on_construct(entt::registry &registry, const entt::entity entt)
    {
        registry.emplace_or_replace<PlayerControllerComponent>(entt);
    }

    void PlayerComponent::on_destroy(entt::registry &registry, const entt::entity entt)
    {
        if (registry.any_of<PlayerControllerComponent>(entt))
        {
            registry.remove<PlayerControllerComponent>(entt);
        }
    }

    void LightComponent::on_construct(entt::registry &registry, const entt::entity entt)
    {
        auto& light_comp = registry.get<LightComponent>(entt);
        auto& transform = registry.get_or_emplace<WorldTransformComponent>(entt);

        LightHandle handle = Engine::get_renderer()->create_light(Light(light_comp, TransformComponent(transform)));
        registry.emplace<LightHandleComponent>(entt, handle);
        LOG_INFO("Added light handle to %d entity", entt);
    }

    void LightComponent::on_destroy(entt::registry &registry, const entt::entity entt)
    {
        if (registry.all_of<LightHandleComponent>(entt))
        {
            registry.remove<LightHandleComponent>(entt);
        }
    }

    void LightHandleComponent::on_destroy(entt::registry &registry, const entt::entity entt)
    {
        auto& handle_comp = registry.get<LightHandleComponent>(entt);
        if (handle_comp.light_handle.is_valid())
        {
            Engine::get_renderer()->destroy_light(handle_comp.light_handle);
        }
    }

    void InteractorComponent::on_construct(entt::registry &registry, const entt::entity entt)
    {
        registry.emplace_or_replace<InteractionControllerComponent>(entt);
    }

    void InteractorComponent::on_destroy(entt::registry &registry, const entt::entity entt)
    {
        if (registry.any_of<InteractionControllerComponent>(entt))
        {
            registry.remove<InteractionControllerComponent>(entt);
        }
    }

    void EditorCameraComponent::on_construct(entt::registry &registry, const entt::entity entt)
    {
        registry.emplace_or_replace<EditorCameraControllerComponent>(entt);
    }

    void EditorCameraComponent::on_destroy(entt::registry &registry, const entt::entity entt)
    {
        if (registry.any_of<EditorCameraControllerComponent>(entt))
        {
            registry.remove<EditorCameraControllerComponent>(entt);
        }
    }
}
