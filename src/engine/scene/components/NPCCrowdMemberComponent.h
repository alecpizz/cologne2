//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/asset_manager/AssetHandle.h>
#include <engine/animation/AnimationClip.h>

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

        //bad, dumb, stupid

        //cool, awesome, sexy
        AssetHandle<AnimationClip> idle_clip;
        AssetHandle<AnimationClip> run_clip;
        AssetHandle<AnimationClip> attack_clip;
        AssetHandle<AnimationClip> hit_clip;
    };
}
