#pragma once
#include <engine/audio/Audio.h>
#include <engine/renderer/Renderer.h>
#include <engine/scene/ScriptableEntity.h>

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
        float _rpm = 60.0f / 350.0f;
        float _reload_time = 0.25f;
        float _shot_timer = 0.0f;
        int _max_ammo = 10;
        int _current_ammo = 0;
        float _gun_time = 0.0f;
        bool _is_firing = false;
        bool _is_reloading = false;
        const char *shoot_sound = RESOURCES_PATH "sounds/vsk_fire.ogg";
        const char *reload_sound = RESOURCES_PATH "sounds/vsk_reload_empty.ogg";
        //view model stuff
        float _time = 0.0f;
        TransformComponent _prev_transform;

        void update_camera(float dt);

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

            auto next_step_time = remap(player_comp.minStepVelocity, player_comp.maxStepVelocity,
                                        player_comp.minStepInterval, player_comp.maxStepInterval, speed);
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

        void move_viewmodel(float dt);

        void apply_friction(float t, float dt);

        void acceleration(glm::vec3 goal_dir, float goal_speed, float accel, float dt);

        void ground_move(glm::vec3 movement_input, float dt);

        void air_move(glm::vec3 movement_input, bool strafing_only, float dt);

        void air_control(glm::vec3 movement_input, float target_speed, bool only_forward, float dt);

        void queue_jump();

        void update_gun(float dt);


    protected:
        void on_create() override;

        void on_destroy() override
        {
        }

        void on_update(float dt) override;

        RuntimeMode get_runtime_mode() override
        {
            return RuntimeMode::GAME_ONLY;
        }
    };
}
