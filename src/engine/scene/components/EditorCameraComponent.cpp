//
// Created by alecpizz on 10/5/25.
//
#include <entt/entt.hpp>
#include "EditorCameraComponent.h"
namespace cologne
    {

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