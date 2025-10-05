//
// Created by alecpizz on 10/5/25.
//
#include <entt/entt.hpp>
#include "InteractorComponent.h"
#include "InteractionControllerComponent.h"
namespace cologne
    {
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
}