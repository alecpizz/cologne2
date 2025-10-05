//
// Created by alecpizz on 10/5/25.
//
#include "StaticColliderComponent.h"
#include <entt/entt.hpp>
#include <engine/physics/Physics.h>
namespace cologne
    {
    void StaticColliderComponent::on_destroy(entt::registry &registry, const entt::entity entt)
    {
        auto col = registry.get<StaticColliderComponent>(entt);
        Physics::destroy_body(col.body_id);
    }
}