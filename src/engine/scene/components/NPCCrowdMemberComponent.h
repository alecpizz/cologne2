//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/asset_manager/AssetHandle.h>
#include <engine/animation/AnimationClip.h>
#include <entt/entt.hpp>

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
            STAGGERING,
            DYING
        };
        State current_state = SPAWNING;
        float state_timer = 0.0f;

        int agent_id = -1;
        glm::vec3 offset = glm::vec3(0.0f, -0.15f, 0.0f);

        float detection_radius = 20.0f;
        float attack_range = 1.5f;
        float attack_cooldown = 1.2f;
        float time_since_last_attack = 0.0f;
        float walk_speed = 1.0f;
        float run_speed = 2.5f;
        float sprint_speed = 4.0f;
        float current_speed = 2.5f;
        float max_health = 100.0f;
        float health = max_health;
        bool was_hit = false;
        AssetHandle<AnimationClip> spawn_clip;
        AssetHandle<AnimationClip> idle_clip;
        AssetHandle<AnimationClip> walk_clip;
        AssetHandle<AnimationClip> run_clip;
        AssetHandle<AnimationClip> sprint_clip;
        AssetHandle<AnimationClip> attack_clip;
        AssetHandle<AnimationClip> stagger_clip;

    };
}
