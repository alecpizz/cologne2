//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <entt/entt.hpp>
namespace cologne
{

    struct StaticColliderComponent
    {
        uint32_t body_id = 0;
        std::string mesh_name;
        bool body_enabled = true;

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

}