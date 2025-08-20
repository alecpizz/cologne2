//
// Created by alecpizz on 8/19/25.
//

#include "InteractionSystem.h"

#include <engine/core/Engine.h>
#include <engine/physics/RaycastHitInfo.h>
#include <engine/renderer/Renderer.h>
#include <engine/scene/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void InteractionSystem::on_update(float dt)
    {
        if (Engine::in_edit_mode())
        {
            return;
        }
        auto &registry = _scene->get_raw_registry();
        auto view = registry.view<InteractorComponent, InteractionControllerComponent, TransformComponent>();
        for (auto &entity: view)
        {
            auto &controller = registry.get<InteractionControllerComponent>(entity);
            auto &interactor = registry.get<InteractorComponent>(entity);
            auto &transform = registry.get<TransformComponent>(entity);
            if (!interactor.update_every_frame)
            {
                continue;
            }
            controller.last_entity = controller.current_entity;
            glm::vec3 ray_start = transform.position, ray_dir = transform.get_forward();
            RaycastHitInfo info;
            if (Physics::raycast(ray_start, ray_dir, 20.0f, Physics::NON_MOVING | Physics::MOVING, info))
            {
                if (Entity hit_entity = info.hit_entity)
                {
                    std::string name = hit_entity.get_component<TagComponent>().tag;
                    Engine::get_renderer()->draw_text(name.c_str(),
                                                      glm::vec3(0.0f, 400.0f, 0.0f),
                                                      glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), .6f);
                    Engine::get_renderer()->draw_text(std::to_string(info.hit_length).c_str(),
                                                      glm::vec3(0.0f, 450.0f, 0.0f),
                                                      glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), .6f);
                    controller.current_entity = static_cast<uint32_t>(hit_entity);
                }
                else
                {
                    controller.current_entity = static_cast<uint32_t>(entt::null);
                }
                Engine::get_renderer()->draw_line(info.hit_point, info.hit_point + info.hit_normal * 0.1f,
                                                  glm::max(info.hit_normal, glm::vec3(0.1f, 0.1f, 0.1f)));
            }
        }
    }
}
