//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/asset_manager/AssetHandle.h>

namespace cologne
{
    struct NPCCrowdMemberComponent
    {
        enum State : int
        {
            SPAWNING,
            IDLE,
            CHASING,
            ATTACKING,
            DYING
        };

        int agent_id = -1;
        glm::vec3 offset = glm::vec3(0.0f, -0.15f, 0.0f);
        float max_acceleration = 3.5f;
        float max_speed = 1.0f;
        float detection_radius = 20.0f;
        float attack_range = 1.5f;
        float attack_cooldown = 1.2f;
        State current_state = SPAWNING;
        float state_timer = 0.0f;

        //todo: handles PLEASE
        std::string idle_clip_name;
        std::string run_clip_name;
        std::string attack_clip_name;
        std::string hit_clip_name;
    };
}
