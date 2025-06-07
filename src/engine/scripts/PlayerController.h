#pragma once
#include <engine/Types.h>
#include <engine/audio/Audio.h>
#include <engine/physics/Physics.h>

namespace cologne
{
    class PlayerController : public ScriptableEntity
    {
    private:
        std::vector<std::string> _footstep_sounds;
        glm::vec2 _rotation = glm::vec2(0.0f);
        bool _is_free_cam = true;
        bool _show_mouse = true;
        bool _allow_sliding = true;
        float _jump_speed = 4.0f;
        float _character_speed = 3.5f;
        bool _was_grounded;
        bool _grounded;
        glm::vec3 _desired_velocity;
        bool _footstep_played = false;
        float _bob_time = 0.0f;
        float _bob_offset = 0.0f;

        void update_camera(float dt)
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
            get_component<PlayerComponent>().camera.get_component<TransformComponent>().rotation = target_rotation;
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
            auto &tr = get_component<PlayerComponent>().camera.get_component<TransformComponent>();
            if (cologne::Input::key_down(Input::Key::LeftShift))
            {
                speed *= 2.5f;
            }
            if (cologne::Input::key_down(Input::Key::W))
            {
                tr.position += fwd * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::S))
            {
                tr.position -= fwd * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::A))
            {
                tr.position += right * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::D))
            {
                tr.position -= right * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::Space))
            {
                tr.position += up * dt * speed;
            }
            if (cologne::Input::key_down(Input::Key::LeftCtrl))
            {
                tr.position -= up * dt * speed;
            }
        }

        void play_footstep(float dt)
        {
            auto vel = Physics::get_player_velocity(get_component<PlayerComponent>().id);
            vel.y = 0.0f;
            if (glm::abs(vel.x) < 0.1f && glm::abs(vel.z) < 0.1f)
            {
                return;
            }

            _bob_time += dt;
            _bob_offset = glm::sin(_bob_time * 4.5f * _character_speed) * 0.05f;
            if (_bob_offset < -0.04f && !_footstep_played && _grounded)
            {
                auto idx = rand() % 4;
                Audio::play_sound(_footstep_sounds[idx].c_str(), 30);
                _footstep_played = true;
            }
            if (_bob_offset > 0.0f)
            {
                _footstep_played = false;
            }

            if (_grounded && !_was_grounded)
            {
                auto idx = rand() % 4;
                Audio::play_sound(_footstep_sounds[idx].c_str(), 30);
                _bob_time = 0.0f;
            }
        }

    protected:
        void on_create() override
        {
            _footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_1.wav");
            _footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_2.wav");
            _footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_3.wav");
            _footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_4.wav");
            for (const auto &footstep_sound: _footstep_sounds)
            {
                Audio::add_sound(footstep_sound.c_str());
            }
        }

        void on_destroy() override
        {
        }

        void on_update(float dt) override
        {
            update_camera(dt);
            float x = 0.0f;
            float y = 0.0f;
            if (cologne::Input::key_down(cologne::Input::Key::W))
            {
                x += 1.0f;
            }
            if (cologne::Input::key_down(cologne::Input::Key::S))
            {
                x -= 1.0f;
            }
            if (cologne::Input::key_down(cologne::Input::Key::A))
            {
                y -= 1.0f;
            }
            if (cologne::Input::key_down(cologne::Input::Key::D))
            {
                y += 1.0f;
            }
            bool jump = cologne::Input::key_pressed(cologne::Input::Key::Space);
            bool crouch = cologne::Input::key_pressed(cologne::Input::Key::LeftCtrl);

            glm::vec3 movement = glm::vec3(-y, 0.0f, x);
            if (abs(x) > 0.0f || abs(y) > 0.0f)
            {
                movement = glm::normalize(movement);
            }

            movement = get_component<PlayerComponent>().camera.get_component<TransformComponent>().rotation * movement;
            movement.y = 0.0f;
            _desired_velocity = movement * _character_speed;

            if (Physics::player_is_supported(get_component<PlayerComponent>().id))
            {
                _allow_sliding = movement.length() > 0.0f;
            } else
            {
                _allow_sliding = false;
            }

            glm::quat up_rotation = glm::quat(glm::vec3(0.0f));
            glm::vec3 up = up_rotation * glm::vec3(0.0f, 1.0f, 0.0f);

            glm::vec3 current_vertical_velocity = glm::dot(Physics::get_player_velocity(
                                                               get_component<PlayerComponent>().id), up) * up;
            glm::vec3 ground_velocity = Physics::get_player_ground_velocity(get_component<PlayerComponent>().id);
            glm::vec3 new_velocity = glm::vec3(0.0f);

            bool moving_towards_ground = current_vertical_velocity.y - ground_velocity.y < 0.1f;
            _was_grounded = _grounded;
            _grounded = Physics::player_is_grounded(get_component<PlayerComponent>().id);
            if (_grounded && !Physics::slope_to_steep_for_player(get_component<PlayerComponent>().id))
            {
                new_velocity = ground_velocity;
                if (jump && moving_towards_ground)
                {
                    new_velocity += _jump_speed * up;
                }
            } else
            {
                new_velocity = current_vertical_velocity;
            }

            new_velocity += (up_rotation * Physics::get_gravity() * dt);
            new_velocity += up_rotation * _desired_velocity;

            PlayerMovementCommand cmd;
            cmd.up = up;
            cmd.rotation = up_rotation;
            cmd.movement = new_velocity;

            Physics::move_player(get_component<PlayerComponent>().id, cmd);
            if (!_is_free_cam)
            {
                play_footstep(dt);
                glm::vec3 player_pos = Physics::get_player_position(
                    get_component<PlayerComponent>().id);
                glm::vec3 camera_pos = player_pos + glm::vec3(
                                    0.0f, 1.45f + _bob_offset, 0.0f);
                get_component<PlayerComponent>().camera.get_component<TransformComponent>().position = camera_pos;
                get_component<TransformComponent>().position = player_pos;
            }
        }
    };
}
