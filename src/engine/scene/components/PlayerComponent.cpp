//
// Created by alecpizz on 10/5/25.
//

#include "PlayerComponent.h"
#include <entt/entt.hpp>
namespace cologne
    {
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

}