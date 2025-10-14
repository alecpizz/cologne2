//
// Created by alecpizz on 8/13/25.
//

#include "BulletSystem.h"

#include <engine/audio/Audio.h>
#include <engine/core/Engine.h>
#include <engine/physics/Physics.h>
#include <engine/scene/components/BulletComponent.h>
#include <engine/scene/components/RigidbodyComponent.h>
#include <engine/scene/Scene.h>
#include <engine/physics/RaycastHitInfo.h>
#include <engine/scene/components/NPCCrowdMemberComponent.h>

namespace cologne
{
    void BulletSystem::on_update(Scene *scene, float dt)
    {
        auto &registry = scene->get_raw_registry();
        auto bullets = registry.view<BulletComponent>();
        RaycastHitInfo info;
        for (auto ent: bullets)
        {
            auto &bullet = registry.get<BulletComponent>(ent);
            if (Physics::raycast(bullet.position, bullet.direction, 20.0f, Physics::NON_MOVING | Physics::MOVING,
                                 info))
            {
                if (info.hit_entity)
                {
                    if (info.hit_entity.has_component<NPCCrowdMemberComponent>())
                    {
                        auto &npc = info.hit_entity.get_component<NPCCrowdMemberComponent>();

                        if (npc.current_state != NPCCrowdMemberComponent::DYING
                            && npc.current_state != NPCCrowdMemberComponent::SPAWNING)
                        {
                            scene->spawn_blood(info.hit_point, bullet.direction);
                            npc.health -= bullet.damage;
                            if (npc.health <= 0)
                            {
                                npc.current_state = NPCCrowdMemberComponent::DYING;
                            }
                            else
                            {
                                //play animation
                            }
                        }
                    }
                    if (info.hit_entity.has_component<RigidbodyComponent>())
                    {
                        auto &rb = info.hit_entity.get_component<RigidbodyComponent>();
                        Physics::add_impulse_force_at_position(rb.body_id, info.hit_point, -info.hit_normal * 20.0f,
                                                               true);
                    }
                }
            }
        }

        for (auto entt: bullets)
        {
            scene->destroy_entity({entt, scene});
        }
    }
}
