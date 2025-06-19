#pragma once
#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include <engine/scene/ScriptableEntity.h>
#include <engine/scene/Components.h>

namespace cologne
{
    class EditorCameraController final : public ScriptableEntity
    {
    private:
        glm::vec2 _rotation = glm::vec2(0.0f);
        bool _was_controlling = false;
        void free_cam(float dt)
        {
            auto mouse = Input::get_relative_mouse();
            constexpr float sensitivity = 30.0f;
            _rotation.x += mouse.x * sensitivity * dt;
            _rotation.y += mouse.y * sensitivity * dt;
            _rotation.y = glm::clamp(_rotation.y, -89.0f, 89.0f);

            glm::quat x_quat = glm::angleAxis(glm::radians(-_rotation.x),
                                              glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat y_quat = glm::angleAxis(glm::radians(_rotation.y),
                                              glm::vec3(1.0f, 0.0f, 0.0f));
            glm::quat target_rotation = x_quat * y_quat;
            get_component<TransformComponent>().rotation = target_rotation;
            glm::vec3 fwd = target_rotation * glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 right = target_rotation * glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 up = target_rotation * glm::vec3(0.0f, 1.0f, 0.0f);

            float speed = 10.0f;
            if (cologne::Input::key_down(Input::Key::LeftShift))
            {
                speed *= 2.5f;
            }
            auto pos = get_component<TransformComponent>().position;
            if (cologne::Input::key_down(Input::Key::W))
            {
                pos += fwd * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::S))
            {
                pos -= fwd * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::A))
            {
                pos += right * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::D))
            {
                pos -= right * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::E))
            {
                pos += up * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::Q))
            {
                pos -= up * dt * speed;
            }
            get_component<TransformComponent>().position = pos;
        }

        void pan(float dt)
        {
            auto mouse = Input::get_relative_mouse();
            const glm::quat target_rotation = get_component<TransformComponent>().rotation;
            const glm::vec3 right = target_rotation * glm::vec3(1.0f, 0.0f, 0.0f);
            const glm::vec3 up = target_rotation * glm::vec3(0.0f, 1.0f, 0.0f);
            auto pos = get_component<TransformComponent>().position;
            constexpr float speed = 5.0f;
            pos += right * dt * mouse.x * speed;
            pos += up * dt * mouse.y * speed;
            get_component<TransformComponent>().position = pos;
        }

    protected:
        void on_create() override
        {
        }

        void on_destroy() override
        {
        }

        void on_update(float dt) override
        {
            if (Input::mouse_down(Input::MouseButton::Middle))
            {
                pan(dt);
            }
            else
            {
                free_cam(dt);
            }

        }

        RuntimeMode get_runtime_mode() override
        {
            return RuntimeMode::EDITOR_ONLY;
        }
    };
}
