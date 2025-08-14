//
// Created by alecpizz on 8/14/25.
//
#pragma once
#include "System.h"

namespace cologne
{
    class PlayerControllerSystem : public System
    {
    public:
        void on_create(Scene *scene) override;

        void on_update(float dt) override;

    private:
        void update_camera(entt::registry &registry, entt::entity entity, float dt);

        void play_footstep(entt::registry &registry, entt::entity entity, float dt);

        void move_viewmodel(entt::registry &registry, entt::entity entity, float dt);

        void apply_friction(entt::registry &registry, entt::entity entity, float t, float dt);

        void acceleration(entt::registry &registry, entt::entity entity, glm::vec3 goal_dir, float goal_speed,
                          float accel, float dt);

        void ground_move(entt::registry &registry, entt::entity entity, glm::vec3 movement_input, float dt);

        void air_move(entt::registry &registry, entt::entity entity, glm::vec3 movement_input, bool strafing_only,
                      float dt);

        void air_control(entt::registry &registry, entt::entity entity, glm::vec3 movement_input, float target_speed,
                         bool only_forward, float dt);

        void queue_jump(entt::registry &registry, entt::entity entity);

        void update_gun(entt::registry &registry, entt::entity entity, float dt);
    };
}
