//
// Created by alecpizz on 8/14/25.
//

#include "EditorCameraControllerSystem.h"

#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include <engine/scene/Components/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void EditorCameraControllerSystem::on_update(Scene* scene, float dt)
    {
        auto& registry = scene->get_raw_registry();
        auto view = registry.view<EditorCameraComponent, TransformComponent, EditorCameraControllerComponent, ActiveComponent>();
        for (auto entity : view)
        {
            if (!registry.get<ActiveComponent>(entity).active)
            {
                continue;
            }
            if (Input::mouse_down(Input::MouseButton::Middle))
            {
                pan(registry, entity, dt);
            }
            else
            {
                free_cam(registry, entity, dt);
            }
        }
    }

    void EditorCameraControllerSystem::free_cam(entt::registry &registry, entt::entity camera, float dt)
    {
        auto& controller = registry.get<EditorCameraControllerComponent>(camera);
        auto& transform = registry.get<TransformComponent>(camera);

        auto mouse = Input::get_relative_mouse();
        constexpr float sensitivity = 30.0f;
        controller.rotation.x += mouse.x * sensitivity * dt;
        controller.rotation.y += mouse.y * sensitivity * dt;
        controller.rotation.y = glm::clamp(controller.rotation.y, -89.0f, 89.0f);

        glm::quat x_quat = glm::angleAxis(glm::radians(-controller.rotation.x),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat y_quat = glm::angleAxis(glm::radians(controller.rotation.y),
                                          glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat target_rotation = x_quat * y_quat;
        transform.rotation = target_rotation;
        glm::vec3 fwd = target_rotation * glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 right = target_rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 up = target_rotation * glm::vec3(0.0f, 1.0f, 0.0f);

        float speed = 10.0f;
        if (key_down(Input::Key::LeftShift))
        {
            speed *= 2.5f;
        }
        auto pos = transform.position;
        if (key_down(Input::Key::W))
        {
            pos += fwd * dt * speed;
        }
        if (key_down(Input::Key::S))
        {
            pos -= fwd * dt * speed;
        }
        if (key_down(Input::Key::A))
        {
            pos += right * dt * speed;
        }
        if (key_down(Input::Key::D))
        {
            pos -= right * dt * speed;
        }
        if (key_down(Input::Key::E))
        {
            pos += up * dt * speed;
        }
        if (key_down(Input::Key::Q))
        {
            pos -= up * dt * speed;
        }
        transform.position = pos;
    }

    void EditorCameraControllerSystem::pan(entt::registry &registry, entt::entity camera, float dt)
    {
        auto mouse = Input::get_relative_mouse();
        auto& transform = registry.get<TransformComponent>(camera);
        const glm::quat target_rotation = transform.rotation;
        const glm::vec3 right = target_rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        const glm::vec3 up = target_rotation * glm::vec3(0.0f, 1.0f, 0.0f);
        auto pos = transform.position;
        constexpr float speed = 5.0f;
        pos += right * dt * mouse.x * speed;
        pos += up * dt * mouse.y * speed;
        transform.position = pos;
    }
}
