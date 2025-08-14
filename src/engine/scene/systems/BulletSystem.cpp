//
// Created by alecpizz on 8/13/25.
//

#include "BulletSystem.h"

#include <engine/animation/AnimatorComponent.h>
#include <engine/audio/Audio.h>
#include <engine/core/Engine.h>
#include <engine/scene/Components.h>
#include <engine/scene/Scene.h>
#include <engine/physics/RaycastHitInfo.h>

namespace cologne
{
    void BulletSystem::on_update(float dt)
    {
        if (Engine::in_edit_mode())
        {
            return;
        }
        auto& registry = _scene->get_raw_registry();
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
                    if (info.hit_entity.has_component<EnemyComponent>())
                    {
                        auto &enemy = info.hit_entity.get_component<EnemyComponent>();
                        enemy.health -= bullet.damage;
                        Audio::play_sound(enemy.hurt_sound.c_str(), 40);
                        if (enemy.health <= 0)
                        {
                            enemy.dead = true;
                            if (info.hit_entity.has_component<AnimatorComponent>())
                            {
                                info.hit_entity.get_component<AnimatorComponent>().to_ragdoll();
                                info.hit_entity.get_component<AnimatorComponent>().take_ragdoll_hit(
                                    info.hit_point, info.hit_normal);
                            }
                        }
                    }
                    if (info.hit_entity.has_component<RigidbodyComponent>())
                    {
                        auto& rb = info.hit_entity.get_component<RigidbodyComponent>();
                        Physics::add_impulse_force_at_position(rb.body_id, info.hit_point, -info.hit_normal * 20.0f, true);
                    }
                }
            }
        }

        for (auto entt: bullets)
        {
            _scene->destroy_entity({entt, _scene});
        }
    }
}
