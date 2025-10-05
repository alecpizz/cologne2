//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
{
    struct EditorCameraComponent
    {
        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

    struct EditorCameraControllerComponent
    {
        glm::vec2 rotation = glm::vec2(0.0f);
    };

}