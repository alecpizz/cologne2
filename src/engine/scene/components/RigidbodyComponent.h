//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/physics/Physics.h>
#include <entt/entt.hpp>
namespace cologne
{
    struct RigidbodyComponent
    {
        uint32_t body_id;

        glm::mat4 get_transform()
        {
            return Physics::get_rigidbody_transform(body_id);
        }

        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };
}