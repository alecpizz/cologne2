//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/asset_manager/AssetHandle.h>
#include <entt/entt.hpp>
namespace cologne
{

    struct StaticColliderComponent
    {
        uint32_t body_id = 0;
        AssetHandle<Mesh> mesh;
        bool body_enabled = true;

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

}