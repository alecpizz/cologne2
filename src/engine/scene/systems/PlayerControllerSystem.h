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
        void on_scene_start(Scene *scene) override;
        void on_update(Scene* scene, float dt) override;
        UpdateFlags get_update_flags() override
        {
            return RUNTIME;
        }

    private:
        void update_camera(Scene *scene, entt::registry &registry, entt::entity entity, float dt);

        void play_footstep(Scene *scene, entt::registry &registry, entt::entity entity, float dt);

        void move_viewmodel(Scene *scene, entt::registry &registry, entt::entity entity, float dt);

        void apply_friction(Scene *scene, entt::registry &registry, entt::entity entity, float t, float dt);

        void acceleration(Scene *scene, entt::registry &registry, entt::entity entity, glm::vec3 goal_dir, float goal_speed,
                          float accel, float dt);

        void ground_move(Scene *scene ,entt::registry &registry, entt::entity entity, glm::vec3 movement_input, float dt);

        void air_move(Scene *scene, entt::registry &registry, entt::entity entity, glm::vec3 movement_input, bool strafing_only,
                      float dt);

        void air_control(Scene *scene, entt::registry &registry, entt::entity entity, glm::vec3 movement_input, float target_speed,
                         bool only_forward, float dt);

        void queue_jump(Scene *scene,entt::registry &registry, entt::entity entity);

        void update_gun(Scene *scene,entt::registry &registry, entt::entity entity, float dt);
    };
}
