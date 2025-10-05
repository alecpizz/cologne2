//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/physics/Physics.h>
#include <engine/core/UUID.h>
#include <entt/entt.hpp>
#include "TransformComponent.h"

namespace cologne
{
      struct PlayerComponent
    {
        uint32_t id = 0;
        UUID camera = {};
        UUID viewmodel = {};
        float gravity = 9.8f * 2.0f;
        float move_speed = 5.0f;
        float run_acceleration = 7.0f;
        float run_deceleration = 3.0f;
        float air_acceleration = 2.0f;
        float air_deceleration = 2.0f;
        float air_control = 0.1f;
        float side_strafe_acceleration = 15.0f;
        float side_strafe_speed = 1.0f;
        float jump_speed = 7.0f;
        float friction = 6.0f;
        float maxStepVelocity = 12.5;
        float minStepVelocity = 2.50f;
        float minStepInterval = 0.150f;
        float maxStepInterval = 1.250f;

        void teleport_to_position(glm::vec3 pos)
        {
            cologne::Physics::teleport_player(id, pos);
        }

        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

    struct PlayerControllerComponent
    {
        std::vector<std::string> footstep_sounds;
        glm::vec2 rotation = glm::vec2(0.0f);
        bool is_free_cam = false;
        bool show_mouse = false;
        bool allow_sliding = true;
        bool was_grounded = false;
        bool grounded = true;
        glm::vec3 desired_velocity;
        bool footstep_played = false;
        float bob_time = 0.0f;
        float bob_offset = 0.0f;
        glm::vec3 velocity = glm::vec3(0.0f);
        bool jump_queued = false;
        float step_timer = 0.0f;
        float step_time = .01f;
        float rpm = 60.0f / 600.0f;
        float reload_time = 0.25f;
        float shot_timer = 0.0f;
        int max_ammo = 10;
        int current_ammo = 0;
        float gun_time = 0.0f;
        bool is_firing = false;
        bool is_reloading = false;
        const char *shoot_sound = ASSETS_PATH "sounds/vsk_fire.ogg";
        const char *reload_sound = ASSETS_PATH "sounds/vsk_reload_empty.ogg";
        //view model stuff
        float time = 0.0f;
        TransformComponent prev_transform;
    };
}