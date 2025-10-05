//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
{
    struct InteractorComponent
    {
        bool update_every_frame = true;

        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };
}