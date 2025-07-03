#pragma once
#include <engine/Types.h>
#include <engine/audio/Audio.h>
#include <engine/physics/Physics.h>
#include <engine/renderer/Renderer.h>

namespace cologne
{
    class PlayerController final : public ScriptableEntity
    {
    private:
        std::vector<std::string> _footstep_sounds;
        glm::vec2 _rotation = glm::vec2(0.0f);
        bool _is_free_cam = false;
        bool _show_mouse = false;
        bool _allow_sliding = true;
        bool _was_grounded = false;
        bool _grounded = true;
        glm::vec3 _desired_velocity;
        bool _footstep_played = false;
        float _bob_time = 0.0f;
        float _bob_offset = 0.0f;
        glm::vec3 _velocity = glm::vec3(0.0f);
        bool _jump_queued = false;
        float _step_timer = 0.0f;
        float _step_time = .01f;

        //view model stuff
        float _time = 0.0f;
        TransformComponent _prev_transform;

        void update_camera(float dt)
        {
            if (cologne::Input::key_pressed(Input::Key::Escape))
            {
                _show_mouse = !_show_mouse;
                if (_show_mouse)
                {
                    Engine::get_window()->show_mouse();
                } else
                {
                    Engine::get_window()->hide_mouse();
                }
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

            bool was_free_cam = _is_free_cam;
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

        float inverse_lerp(float a, float b, float v)
        {
            return (v - a) / (b - a);
        }

        float lerp(float a, float b, float t)
        {
            return (1.0f - t) * a + b * t;
        }

        float remap(float in_min, float in_max, float out_min, float out_max, float v)
        {
            float t = inverse_lerp(in_min, in_max, v);
            return lerp(out_min, out_max, t);
        }

        void play_footstep(float dt)
        {
            if (!_grounded)
            {
                return;
            }
            auto vel = _velocity;
            vel.y = 0.0f;
            auto speed = glm::length(vel);
            auto player_comp = get_component<PlayerComponent>();
            if (speed < player_comp.minStepVelocity)
            {
                return;
            }

            auto next_step_time = remap(player_comp.minStepVelocity, player_comp.maxStepVelocity, player_comp.minStepInterval, player_comp.maxStepInterval, speed);
            if (_step_timer > _step_time)
            {
                _step_time = next_step_time;
                _step_timer = 0.0f;
                auto idx = rand() % 4;
                Audio::play_sound(_footstep_sounds[idx].c_str(), glm::linearRand(25, 35));
            }
            else
            {
                _step_timer += dt;
            }
        }

        void move_viewmodel(float dt)
        {
            glm::vec2 mouse = Input::get_relative_mouse();
            auto &viewmodel = get_component<PlayerComponent>().viewmodel.get_component<ViewmodelComponent>();
            float mouse_x = mouse.x * viewmodel.sway_multiplier * dt;
            float mouse_y = mouse.y * viewmodel.sway_multiplier * dt;

            glm::quat x_rotation = glm::angleAxis(glm::radians(-mouse_y), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::quat y_rotation = glm::angleAxis(glm::radians(-mouse_x), glm::vec3(0.0, 1.0f, 0.0f));
            glm::quat target_rotation = x_rotation * y_rotation;
            glm::quat new_rotation = glm::slerp(_prev_transform.rotation,
                                                target_rotation * glm::quat(
                                                    glm::radians(glm::vec3(viewmodel.euler_offset))),
                                                dt * viewmodel.smoothing);
            glm::vec3 velocity = Physics::get_player_velocity(get_component<PlayerComponent>().id);
            float y_vel = velocity.y;
            velocity.y = 0.0f;
            if (glm::length2(velocity) > 0.0f)
            {
                _time += dt;
            } else
            {
                _time = 0.0f;
            }

            glm::vec3 bob = glm::vec3(0.0f);
            bob.y += glm::sin(_time * viewmodel.frequency) * viewmodel.amplitude;
            bob.x += glm::cos(_time * viewmodel.frequency / 2.0f) * viewmodel.amplitude * 2.0f;
            bob.y += glm::clamp(-y_vel * viewmodel.vertical_velocity_multiplier,
                                -viewmodel.max_vertical_offset, viewmodel.max_vertical_offset);
            glm::vec3 new_position = glm::lerp(_prev_transform.position,
                                               bob + viewmodel.position_offset, dt * viewmodel.smoothing);
            _prev_transform.position = new_position;
            _prev_transform.rotation = new_rotation;

            glm::mat4 gun_mat = glm::mat4(1.0f);
            auto &cam_transform = get_component<PlayerComponent>().camera.get_component<TransformComponent>();
            gun_mat = glm::translate(gun_mat, new_position);
            gun_mat *= glm::toMat4(new_rotation);
            gun_mat = glm::inverse(Renderer::get_camera_view(cam_transform)) * gun_mat;
            glm::quat orientation;
            glm::vec3 translation;
            glm::vec3 scale;
            glm::vec4 persp;
            glm::vec3 skew;
            glm::decompose(gun_mat, scale, orientation, translation, skew, persp);

            get_component<PlayerComponent>().viewmodel.get_component<TransformComponent>().position = translation;
            get_component<PlayerComponent>().viewmodel.get_component<TransformComponent>().rotation = orientation;
        }

        void apply_friction(float t, float dt)
        {
            glm::vec3 v = _velocity;
            v.y = 0.0f;
            float speed = length(v);
            float drop = 0.0f;
            if (_grounded)
            {
                float control = speed < get_component<PlayerComponent>().run_deceleration ? get_component<PlayerComponent>().run_deceleration : speed;
                drop = control * get_component<PlayerComponent>().friction * dt * t;
            }

            float new_speed = speed - drop;
            if (new_speed < 0.0f)
            {
                new_speed = 0.0f;
            }
            if (new_speed > 0.0f)
            {
                new_speed /= speed;
            }

            _velocity.x *= new_speed;
            _velocity.z *= new_speed;
        }

        void acceleration(glm::vec3 goal_dir, float goal_speed, float accel, float dt)
        {
            float current_speed = glm::dot(_velocity, goal_dir);
            float add_speed = goal_speed - current_speed;
            if (add_speed <= 0)
            {
                return;
            }

            float accel_speed = accel * dt * goal_speed;
            if (accel_speed > add_speed)
            {
                accel_speed = add_speed;
            }
            _velocity.x += accel_speed * goal_dir.x;
            _velocity.z += accel_speed * goal_dir.z;
        }

        void ground_move(glm::vec3 movement_input, float dt)
        {
            apply_friction(!_jump_queued ? 1.0f : 0.0f, dt);
            if (length2(movement_input) != 0.0f)
            {
                movement_input = normalize(movement_input);
            }
            float goal_speed = length(movement_input) * get_component<PlayerComponent>().move_speed;
            acceleration(movement_input, goal_speed, get_component<PlayerComponent>().run_acceleration, dt);
            _velocity.y = -get_component<PlayerComponent>().gravity * dt;
            if (_jump_queued)
            {
                _velocity.y = get_component<PlayerComponent>().jump_speed;
                _jump_queued = false;
            }
            else
            {
                //slope correct here
            }
        }

        void air_move(glm::vec3 movement_input, bool strafing_only, float dt)
        {
            float accel;
            float wish_speed = glm::length(movement_input);
            wish_speed *= get_component<PlayerComponent>().move_speed;
            if (length(movement_input) != 0.0f)
            {
                movement_input = normalize(movement_input);
            }

            float wish_speed2 = wish_speed;
            if (glm::dot(_velocity, movement_input) < 0)
            {
                accel = get_component<PlayerComponent>().air_deceleration;
            }
            else
            {
                accel = get_component<PlayerComponent>().air_acceleration;
            }

            if (strafing_only)
            {
                if (wish_speed > get_component<PlayerComponent>().side_strafe_speed)
                {
                    wish_speed = get_component<PlayerComponent>().side_strafe_speed;
                }
                accel = get_component<PlayerComponent>().side_strafe_acceleration;
            }
            acceleration(movement_input, wish_speed, accel, dt);
            if (get_component<PlayerComponent>().air_control > 0)
            {
                air_control(movement_input, wish_speed2, !strafing_only, dt);
            }
            _velocity.y -= get_component<PlayerComponent>().gravity * dt;
        }

        void air_control(glm::vec3 movement_input, float target_speed, bool only_forward, float dt)
        {
            if (!only_forward || glm::abs(target_speed) < 0.0001f)
            {
                return;
            }

            float z_speed = _velocity.y;
            _velocity.y = 0.0f;

            float speed = length(_velocity);
            if (speed != 0.0f)
            {
                _velocity = glm::normalize(_velocity);
            }

            float dot = glm::dot(_velocity, movement_input);
            float k = 32;
            k *= get_component<PlayerComponent>().air_control * dot * dot * dt;

            if (dot > 0)
            {
                _velocity *= speed * glm::length(movement_input) * k;
                _velocity = glm::normalize(_velocity);
            }
            _velocity.x *= speed;
            _velocity.y = z_speed;
            _velocity.z *= speed;
        }

        void queue_jump()
        {
            if (Input::key_pressed(Input::Key::Space))
            {
                _jump_queued = true;
            }
            if (!Input::key_pressed(Input::Key::Space))
            {
                _jump_queued = false;
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
            if (_is_free_cam)
            {
                get_component<PlayerComponent>().teleport_to_position(
                    get_component<PlayerComponent>().camera.get_component<TransformComponent>().position
                    - glm::vec3(0.0f, 1.45f, 0.0f));
                move_viewmodel(dt);
                return;
            }
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
            queue_jump();
            bool crouch = cologne::Input::key_pressed(cologne::Input::Key::LeftCtrl);

            glm::vec3 movement = glm::vec3(-y, 0.0f, x);
            movement = get_component<PlayerComponent>().camera.get_component<TransformComponent>().rotation * movement;
            movement.y = 0.0f;
            if (abs(movement.x) > 0.0f || abs(movement.y) > 0.0f)
            {
                movement = glm::normalize(movement);
            }


            glm::quat up_rotation = glm::quat(glm::vec3(0.0f));
            glm::vec3 up = up_rotation * glm::vec3(0.0f, 1.0f, 0.0f);



            _was_grounded = _grounded;
            _grounded = Physics::player_is_grounded(get_component<PlayerComponent>().id);
            if (_grounded)
            {
                ground_move(movement, dt);
            }
            else
            {
                //air move
                bool strafing_only = y == 0 && x != 0;
                air_move(movement, !strafing_only, dt);
            }

            PlayerMovementCommand cmd;
            cmd.up = up;
            cmd.rotation = up_rotation;
            cmd.movement = _velocity;

            Physics::move_player(get_component<PlayerComponent>().id, cmd);

            play_footstep(dt);
            glm::vec3 player_pos = Physics::get_player_position(
                get_component<PlayerComponent>().id);
            glm::vec3 camera_pos = player_pos + glm::vec3(0.0f, 1.45f, 0.0f);
            get_component<PlayerComponent>().camera.get_component<TransformComponent>().position = camera_pos;
            get_component<TransformComponent>().position = player_pos;
            move_viewmodel(dt);
        }

        RuntimeMode get_runtime_mode() override
        {
            return RuntimeMode::GAME_ONLY;
        }
    };
}
