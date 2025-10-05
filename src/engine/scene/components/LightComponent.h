//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/renderer/types/LightHandle.h>
#include <entt/entt.hpp>
namespace cologne
{

    struct LightComponent
    {
        enum LightType
        {
            Directional = 0,
            Point = 1,
            Spot = 2
        };

        glm::vec3 color = glm::vec4(1, 0.7799999713897705, 0.5289999842643738, 1.0f);
        float strength = 1.0f;
        float radius = 3.0f;
        int type = LightType::Point;
        float outer_cutoff = 17.5f;
        float inner_cutoff = 12.5f;
        bool cast_shadows = false;
        bool always_update_shadows = true;

        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

    struct LightHandleComponent
    {
        LightHandle light_handle;

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };
}