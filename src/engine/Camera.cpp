//
// Created by alecp on 3/14/2025.
//

#include "Camera.h"

#include "Engine.h"
#include "Input.h"

namespace goon
{
    Camera::Camera(glm::vec3 position, glm::vec3 forward, glm::vec3 up)
    {
        _position = position;
        _forward = forward;
        _up = up;
    }

    glm::vec3 Camera::get_position() const
    {
        return _position;
    }

    glm::vec3 Camera::get_forward() const
    {
        return _forward;
    }

    glm::vec3 Camera::get_up() const
    {
        return _up;
    }

    glm::mat4 Camera::get_view_matrix() const
    {
        return glm::lookAt(_position, _position + _forward, _up);
    }

    glm::mat4 Camera::get_projection_matrix() const
    {
        return glm::perspective(fov,
            static_cast<float>(Engine::get_window()->get_width()) /
            static_cast<float>(Engine::get_window()->get_height()),
            0.1f, 500.0f);
    }

    void Camera::update(float dt)
    {
        if (goon::Input::key_pressed(Input::Key::Escape))
        {
            _active = !_active;
            if (!_active)
            {
                Engine::get_window()->show_mouse();
            } else
            {
                Engine::get_window()->hide_mouse();
            }
        }
        if (!_active)
        {
            return;
        }
        if (goon::Input::key_down(Input::Key::W))
        {
            _position += _forward * dt * 10.0f;
        }
        if (goon::Input::key_down(Input::Key::S))
        {
            _position -= _forward * dt * 10.0f;
        }
        if (goon::Input::key_down(Input::Key::A))
        {
            _position -= glm::normalize(glm::cross(_forward, _up)) * dt * 10.0f;
        }
        if (goon::Input::key_down(Input::Key::D))
        {
            _position += glm::normalize(glm::cross(_forward, _up)) * dt * 10.0f;
        }

        glm::vec2 mouse = Input::get_relative_mouse();

        _yaw += mouse.x * dt * 10.0f;
        _pitch -= mouse.y * dt * 10.0f;
        if (_pitch > 89.0f)
        {
            _pitch = 89.0f;
        }
        if (_pitch < -89.0f)
        {
            _pitch = -89.0f;
        }
        glm::vec3 front;
        front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front.y = sin(glm::radians(_pitch));
        front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _forward = glm::normalize(front);
    }
}
