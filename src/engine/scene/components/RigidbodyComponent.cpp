//
// Created by alecpizz on 10/5/25.
//
#include "RigidbodyComponent.h"
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
}