//
// Created by alecpizz on 10/5/25.
//
#include <entt/entt.hpp>
#include "LightComponent.h"
#include "WorldTransformComponent.h"
#include "TransformComponent.h"
#include <engine/renderer/Renderer.h>
#include <engine/core/Engine.h>
namespace cologne
    {

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

}