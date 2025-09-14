//
// Created by alecpizz on 9/14/25.
//

#include "NavigationSystem.h"

#include <engine/navigation/Navigation.h>
#include <engine/renderer/Renderer.h>
#include <engine/scene/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void NavigationSystem::on_scene_start(Scene *scene)
    {
        auto& registry = scene->get_raw_registry();
        auto view = registry.view<TransformComponent, NPCMemberComponent>();
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            auto& npc_controller = view.get<NPCMemberComponent>(entity);

            int agent_id = Navigation::add_agent(transform.position);
            npc_controller.agent_id = agent_id;
        }
    }

    void NavigationSystem::on_update(Scene *scene, float dt)
    {
        auto& registry = scene->get_raw_registry();
        auto player_view = registry.view<PlayerComponent>();
        entt::entity player_temp;
        for (auto entity : player_view)
        {
            player_temp = entity;
            break;
        }

        auto enemy_view = registry.view<EnemyComponent>();
        auto player_pos = registry.get<TransformComponent>(player_temp).position;
        Renderer::draw_point(player_pos, glm::vec3(0.0, 0.0, 1.0));
        for (auto entity : enemy_view)
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

        auto npc_view = registry.view<TransformComponent, NPCMemberComponent>();
        for (auto entity : npc_view)
        {
            auto& transform = npc_view.get<TransformComponent>(entity);
            auto& npc_controller = npc_view.get<NPCMemberComponent>(entity);

            if (npc_controller.agent_id != -1)
            {
                Navigation::set_agent_target(npc_controller.agent_id, target_pos);
                glm::vec3 new_pos = Navigation::get_agent_position(npc_controller.agent_id);
                glm::vec3 look = glm::normalize( new_pos - player_pos);
                look.y = 0.0f;
                glm::quat new_rot = glm::quatLookAt(look, glm::vec3(0.0f, 1.0f, 0.0f));
                transform.position = new_pos;
                transform.rotation = new_rot;
            }
        }
    }
}
