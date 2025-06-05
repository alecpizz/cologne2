#pragma once
#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include <engine/scene/ScriptableEntity.h>
#include <engine/scene/Components.h>

namespace cologne
{
    class CameraController : public ScriptableEntity
    {
    private:
        glm::vec2 _rotation = glm::vec2(0.0f);
        glm::vec3 _position = glm::vec3(0.0f);
        bool _is_free_cam = true;
        bool _show_mouse = true;

    protected:
        void on_create() override
        {
        }

        void on_destroy() override
        {
        }

        void on_update(float dt) override
        {
            if (cologne::Input::key_pressed(Input::Key::Escape))
            {
                _show_mouse = !_show_mouse;
                if (!_show_mouse)
                {
                    Engine::get_window()->show_mouse();
                } else
                {
                    Engine::get_window()->hide_mouse();
                }
            }

            if (Engine::get_event_manager()->paused())
            {
                return;
            }

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

            if (Input::key_pressed(Input::Key::F))
            {
                _is_free_cam = !_is_free_cam;
            }

            if (!_is_free_cam)
            {
                return;
            }

            float speed = 10.0f;
            if (cologne::Input::key_down(Input::Key::LeftShift))
            {
                speed *= 2.5f;
            }
            if (cologne::Input::key_down(Input::Key::W))
            {
                _position += fwd * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::S))
            {
                _position -= fwd * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::A))
            {
                _position += right * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::D))
            {
                _position -= right * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::Space))
            {
                _position += up * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::LeftCtrl))
            {
                _position -= up * dt * speed;
            }
            get_component<TransformComponent>().position = _position;
        }
    };
}
