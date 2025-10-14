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
            anim.play(npc.idle_clip, 0, true);
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
        if (!registry.valid(player_temp))
        {
            return;
        }

        auto enemy_view = registry.view<EnemyComponent>();
        auto player_pos = registry.get<TransformComponent>(player_temp).position;
        Renderer::draw_point(player_pos, glm::vec3(0.0, 0.0, 1.0));


        Navigation::update_crowd(dt);

        glm::vec3 target_pos = player_pos;

        auto npc_view = registry.view<TransformComponent, NPCCrowdMemberComponent, AnimatorComponent, RagdollComponent,
            EnemyComponent>();
        for (auto entity: npc_view)
        {
            auto &transform = npc_view.get<TransformComponent>(entity);
            auto &npc = npc_view.get<NPCCrowdMemberComponent>(entity);
            auto &animator = npc_view.get<AnimatorComponent>(entity);
            auto &ragdoll = npc_view.get<RagdollComponent>(entity);
            auto &enemy = npc_view.get<EnemyComponent>(entity);

            if (npc.agent_id == -1)
            {
                continue;
            }

            if (npc.current_state != NPCCrowdMemberComponent::SPAWNING && npc.current_state !=
                NPCCrowdMemberComponent::DYING)
            {
                glm::vec3 direction = glm::normalize(transform.position - player_pos);
                direction.y = 0.0f;
                transform.rotation = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f));
            }


            switch (npc.current_state)
            {
                case NPCCrowdMemberComponent::SPAWNING:
                {
                    if (animator.layers[0].clip != npc.spawn_clip)
                    {
                        animator.play(npc.spawn_clip, 0, false);
                    }
                    if (animator.layers[0].is_finished)
                    {
                        LOG_INFO("I SPAWNED, CHASE TIME");
                        npc.current_state = NPCCrowdMemberComponent::CHASING;
                    }
                    break;
                }

                case NPCCrowdMemberComponent::CHASING:
                {
                    float dist_to_player = glm::distance(transform.position, player_pos);
                    animator.crossfade_to(npc.run_clip, 0.3f);

                    //TODO: SET SPEED HERE

                    if (dist_to_player <= npc.attack_range)
                    {
                        LOG_INFO("CAUGHT YOU, IM GONNA ATTACK");
                        npc.current_state = NPCCrowdMemberComponent::ATTACKING;
                        npc.time_since_last_attack = npc.attack_cooldown;
                    }
                    else
                    {
                        Navigation::set_agent_target(npc.agent_id, player_pos);
                    }
                    break;
                }
                case NPCCrowdMemberComponent::ATTACKING:
                {
                    //set speed to zero here
                    Navigation::set_agent_target(npc.agent_id, transform.position);
                    npc.time_since_last_attack += dt;

                    if (npc.time_since_last_attack >= npc.attack_cooldown)
                    {
                        if (animator.layers[0].clip != npc.attack_clip || animator.layers[0].is_finished)
                        {
                            LOG_INFO("GET ATTACKED");
                            animator.play(npc.attack_clip, 0,  false);
                            npc.time_since_last_attack = 0.0f;
                            //TODO: damage
                        }
                    }

                    if (animator.layers[0].is_finished)
                    {
                        float dist_to_player = glm::distance(transform.position, player_pos);
                        if (dist_to_player > npc.attack_range)
                        {
                            LOG_INFO("LOST YOU");
                            npc.current_state = NPCCrowdMemberComponent::CHASING;
                            break;
                        }
                    }


                    break;
                }
                case NPCCrowdMemberComponent::IDLE:
                {
                    animator.crossfade_to(npc.idle_clip, 0.5f);
                    //TODO: set speed
                    Navigation::set_agent_target(npc.agent_id, transform.position);
                    if (glm::distance(transform.position, player_pos) < npc.detection_radius)
                    {
                        npc.current_state = NPCCrowdMemberComponent::CHASING;
                    }
                    break;
                }
                case NPCCrowdMemberComponent::DYING:
                {
                    ragdoll.to_ragdoll();
                    //set speed here
                    Navigation::set_agent_target(npc.agent_id, transform.position);
                    break;
                }
                default: break;
            }

            transform.position = Navigation::get_agent_position(npc.agent_id) + npc.offset;
        }
    }
}
