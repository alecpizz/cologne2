//
// Created by alecpizz on 9/14/25.
//

#include "NPCSystem.h"

#include <engine/navigation/Navigation.h>
#include <engine/renderer/Renderer.h>
#include <engine/scene/components/NPCCrowdMemberComponent.h>
#include <engine/scene/Scene.h>
#include <engine/scene/components/AnimatorComponent.h>
#include <engine/scene/components/EnemyComponent.h>
#include <engine/scene/components/PlayerComponent.h>
#include <engine/scene/components/RagdollComponent.h>

namespace cologne
{
    void NPCSystem::on_scene_start(Scene *scene)
    {
        auto &registry = scene->get_raw_registry();
        auto view = registry.view<TransformComponent, NPCCrowdMemberComponent>();
        for (auto entity: view)
        {
            auto &transform = view.get<TransformComponent>(entity);
            auto &npc_controller = view.get<NPCCrowdMemberComponent>(entity);

            int agent_id = Navigation::add_agent(transform.position, npc_controller);
            npc_controller.agent_id = agent_id;
        }

        auto view2 = registry.view<NPCCrowdMemberComponent, AnimatorComponent>();
        for (auto entity: view2)
        {
           auto [npc, anim] = registry.get<NPCCrowdMemberComponent, AnimatorComponent>(entity);
            anim.play(npc.idle_clip_name, 0, true);
        }
    }

    void NPCSystem::on_update(Scene *scene, float dt)
    {
        auto &registry = scene->get_raw_registry();
        auto player_view = registry.view<PlayerComponent>();
        entt::entity player_temp;
        for (auto entity: player_view)
        {
            player_temp = entity;
            break;
        }

        auto enemy_view = registry.view<EnemyComponent>();
        auto player_pos = registry.get<TransformComponent>(player_temp).position;
        Renderer::draw_point(player_pos, glm::vec3(0.0, 0.0, 1.0));
        for (auto entity: enemy_view)
        {
            auto enemy_pos = registry.get<TransformComponent>(entity).position;
            Renderer::draw_point(enemy_pos, glm::vec3(0.0, 0.0, 1.0));
            std::vector<glm::vec3> path = Navigation::find_path(enemy_pos, player_pos);
            if (path.size() < 2)
            {
                continue;
            }
            for (size_t i = 0; i < path.size() - 1; i++)
            {
                Renderer::draw_line(path[i], path[i + 1], glm::vec3(0.0, 1.0, 0.0));
            }
        }

        Navigation::update_crowd(dt);

        glm::vec3 target_pos = player_pos;

        auto npc_view = registry.view<TransformComponent, NPCCrowdMemberComponent, AnimatorComponent, RagdollComponent,
            EnemyComponent>();
        for (auto entity: npc_view)
        {
            auto &transform = npc_view.get<TransformComponent>(entity);
            auto &npc_controller = npc_view.get<NPCCrowdMemberComponent>(entity);
            auto &animator = npc_view.get<AnimatorComponent>(entity);
            auto &ragdoll = npc_view.get<RagdollComponent>(entity);
            auto &enemy = npc_view.get<EnemyComponent>(entity);

            if (npc_controller.agent_id == -1 || npc_controller.current_state == NPCCrowdMemberComponent::State::DYING)
            {
                continue;
            }

            if (enemy.health <= 0)
            {
                npc_controller.current_state = NPCCrowdMemberComponent::State::DYING;
                ragdoll.to_ragdoll();
                Navigation::remove_agent(npc_controller.agent_id);
                continue;
            }

            npc_controller.state_timer += dt;
            float distance_to_player = glm::distance(transform.position, player_pos);
            switch (npc_controller.current_state)
            {
                case NPCCrowdMemberComponent::State::IDLE:
                    animator.crossfade_to(npc_controller.idle_clip_name, 0.2f, 1);
                    if (distance_to_player < npc_controller.detection_radius)
                    {
                        npc_controller.current_state = NPCCrowdMemberComponent::State::CHASING;
                    }
                    break;
                case NPCCrowdMemberComponent::State::CHASING:
                    animator.crossfade_to(npc_controller.run_clip_name, 0.2f, 1);
                    Navigation::set_agent_target(npc_controller.agent_id, player_pos);
                    if (distance_to_player <= npc_controller.attack_range)
                    {
                        npc_controller.current_state = NPCCrowdMemberComponent::State::ATTACKING;
                        npc_controller.state_timer = 0.0f;
                    }
                    break;
                case NPCCrowdMemberComponent::State::ATTACKING:
                    Navigation::set_agent_target(npc_controller.agent_id, transform.position);
                    if (npc_controller.state_timer >= npc_controller.attack_cooldown)
                    {
                        animator.play(npc_controller.attack_clip_name, 1, false);
                        if (distance_to_player > npc_controller.attack_range)
                        {
                            npc_controller.current_state = NPCCrowdMemberComponent::State::CHASING;
                        }
                        else
                        {
                            npc_controller.state_timer = 0.0f;
                        }
                    }
                    break;
                default:
                    break;
            }

            glm::vec3 new_pos = Navigation::get_agent_position(npc_controller.agent_id) + npc_controller.offset;
            glm::vec3 look = glm::normalize(new_pos - player_pos);
            look.y = 0.0f;
            glm::quat new_rot = glm::quatLookAt(look, glm::vec3(0.0f, 1.0f, 0.0f));
            transform.position = new_pos;
            transform.rotation = new_rot;
        }
    }
}
