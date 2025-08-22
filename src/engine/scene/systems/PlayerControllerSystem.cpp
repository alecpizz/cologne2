//
// Created by alecpizz on 8/14/25.
//

#include "PlayerControllerSystem.h"

#include <engine/animation/AnimatorComponent.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/audio/Audio.h>
#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer.h>
#include <engine/scene/Components.h>
#include <engine/scene/Entity.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void PlayerControllerSystem::on_create()
    {
        System::on_create();
        
    }

    void PlayerControllerSystem::on_scene_start(Scene *scene)
    {
        auto &registry = scene->get_raw_registry();
        auto view = registry.view<PlayerComponent, PlayerControllerComponent>();
        for (auto entity: view)
        {
            auto &controller = registry.get<PlayerControllerComponent>(entity);
            if (controller.footstep_sounds.empty())
            {
                controller.footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_1.wav");
                controller.footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_2.wav");
                controller.footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_3.wav");
                controller.footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_4.wav");
            }
            for (const auto &footstep_sound: controller.footstep_sounds)
            {
                Audio::add_sound(footstep_sound.c_str());
            }
            auto anim = AssetManager::get_animation_by_name("deagle_Rig|Rig|MK_ReloadFull");
            controller.reload_time = anim->get_duration() / anim->get_ticks_per_second();
            Audio::add_sound(controller.shoot_sound);
            Audio::add_sound(controller.reload_sound);
            controller.current_ammo = controller.max_ammo;
        }
    }

    void PlayerControllerSystem::on_update(Scene *scene, float dt)
    {
        auto &registry = scene->get_raw_registry();
        auto view = registry.view<PlayerComponent, PlayerControllerComponent, TransformComponent>();
        for (auto entity: view)
        {
            auto &controller = registry.get<PlayerControllerComponent>(entity);
            auto &player = registry.get<PlayerComponent>(entity);
            update_camera(scene, registry, entity, dt);
            if (controller.is_free_cam)
            {
                player.teleport_to_position(
                    scene->get_entity_by_uuid(player.camera)
                    .get_transform().position
                    - glm::vec3(0.0f, 1.45f, 0.0f));
                move_viewmodel(scene, registry, entity, dt);
                continue;
            }
            float x = 0.0f;
            float y = 0.0f;
            if (key_down(Input::Key::W))
            {
                x += 1.0f;
            }
            if (key_down(Input::Key::S))
            {
                x -= 1.0f;
            }
            if (key_down(Input::Key::A))
            {
                y -= 1.0f;
            }
            if (key_down(Input::Key::D))
            {
                y += 1.0f;
            }
            queue_jump(scene, registry, entity);
            bool crouch = cologne::Input::key_pressed(cologne::Input::Key::LeftCtrl);

            glm::vec3 movement = glm::vec3(-y, 0.0f, x);
            movement = scene->get_entity_by_uuid(player.camera)
                       .get_transform().rotation * movement;
            movement.y = 0.0f;
            if (abs(movement.x) > 0.0f || abs(movement.y) > 0.0f)
            {
                movement = glm::normalize(movement);
            }


            glm::quat up_rotation = glm::quat(glm::vec3(0.0f));
            glm::vec3 up = up_rotation * glm::vec3(0.0f, 1.0f, 0.0f);


            controller.was_grounded = controller.grounded;
            controller.grounded = Physics::player_is_grounded(player.id);
            if (controller.grounded)
            {
                ground_move(scene,registry, entity, movement, dt);
            }
            else
            {
                //air move
                bool strafing_only = y == 0 && x != 0;
                air_move(scene,registry, entity, movement, !strafing_only, dt);
            }

            PlayerMovementCommand cmd;
            cmd.up = up;
            cmd.rotation = up_rotation;
            cmd.movement = controller.velocity;

            Physics::move_player(player.id, cmd);

            play_footstep(scene, registry, entity, dt);
            glm::vec3 player_pos = Physics::get_player_position(
                player.id);
            glm::vec3 camera_pos = player_pos + glm::vec3(0.0f, 1.45f, 0.0f);
            scene->get_entity_by_uuid(player.camera)
                    .get_transform().position = camera_pos;
            registry.get<TransformComponent>(entity).position = player_pos;
            update_gun(scene, registry, entity, dt);
            move_viewmodel(scene, registry, entity, dt);
        }
    }

    void PlayerControllerSystem::update_camera(Scene *scene, entt::registry &registry, entt::entity entity, float dt)
    {
        auto &controller = registry.get<PlayerControllerComponent>(entity);
        auto &player = registry.get<PlayerComponent>(entity);
        if (cologne::Input::key_pressed(Input::Key::Escape))
        {
            controller.show_mouse = !controller.show_mouse;
            if (controller.show_mouse)
            {
                Engine::get_window()->show_mouse();
            }
            else
            {
                Engine::get_window()->hide_mouse();
            }
        }
        auto mouse = Input::get_relative_mouse();
        constexpr float sensitivity = 30.0f;
        controller.rotation.x += mouse.x * sensitivity * dt;
        controller.rotation.y += mouse.y * sensitivity * dt;
        controller.rotation.y = glm::clamp(controller.rotation.y, -89.0f, 89.0f);
        glm::quat x_quat = glm::angleAxis(glm::radians(-controller.rotation.x),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat y_quat = glm::angleAxis(glm::radians(controller.rotation.y),
                                          glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat target_rotation = x_quat * y_quat;
        Entity camera = scene->get_entity_by_uuid(player.camera);
        camera.get_transform().rotation = target_rotation;
        glm::vec3 fwd = target_rotation * glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 right = target_rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 up = target_rotation * glm::vec3(0.0f, 1.0f, 0.0f);

        bool was_free_cam = controller.is_free_cam;
        if (Input::key_pressed(Input::Key::F))
        {
            controller.is_free_cam = !controller.is_free_cam;
        }

        if (!controller.is_free_cam)
        {
            return;
        }

        float speed = 10.0f;
        auto &tr = camera.get_transform();
        if (cologne::Input::key_down(Input::Key::LeftShift))
        {
            speed *= 2.5f;
        }
        if (cologne::Input::key_down(Input::Key::W))
        {
            tr.position += fwd * dt * speed;
        }
        if (cologne::Input::key_down(Input::Key::S))
        {
            tr.position -= fwd * dt * speed;
        }
        if (cologne::Input::key_down(Input::Key::A))
        {
            tr.position += right * dt * speed;
        }
        if (cologne::Input::key_down(Input::Key::D))
        {
            tr.position -= right * dt * speed;
        }
        if (cologne::Input::key_down(Input::Key::Space))
        {
            tr.position += up * dt * speed;
        }
        if (cologne::Input::key_down(Input::Key::LeftCtrl))
        {
            tr.position -= up * dt * speed;
        }
    }

    void PlayerControllerSystem::play_footstep(Scene *scene, entt::registry &registry, entt::entity entity, float dt)
    {
        auto &controller = registry.get<PlayerControllerComponent>(entity);
        auto &player = registry.get<PlayerComponent>(entity);
        if (!controller.grounded)
        {
            return;
        }
        auto vel = controller.velocity;
        vel.y = 0.0f;
        auto speed = glm::length(vel);
        if (speed < player.minStepVelocity)
        {
            return;
        }

        auto next_step_time = Util::remap(player.minStepVelocity, player.maxStepVelocity,
                                          player.minStepInterval, player.maxStepInterval, speed);
        if (controller.step_timer > controller.step_time)
        {
            controller.step_time = next_step_time;
            controller.step_timer = 0.0f;
            auto idx = rand() % 4;
            Audio::play_sound(controller.footstep_sounds[idx].c_str(), glm::linearRand(25, 35));
        }
        else
        {
            controller.step_timer += dt;
        }
    }

    void PlayerControllerSystem::move_viewmodel(Scene *scene, entt::registry &registry, entt::entity entity, float dt)
    {
        auto &controller = registry.get<PlayerControllerComponent>(entity);
        auto &player = registry.get<PlayerComponent>(entity);
        glm::vec2 mouse = Input::get_relative_mouse();
        Entity viewmodel_entity = scene->get_entity_by_uuid(
            player.viewmodel);
        auto &viewmodel = viewmodel_entity.get_component<ViewmodelComponent>();
        float mouse_x = mouse.x * viewmodel.sway_multiplier * dt;
        float mouse_y = mouse.y * viewmodel.sway_multiplier * dt;

        glm::quat x_rotation = glm::angleAxis(glm::radians(-mouse_y), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat y_rotation = glm::angleAxis(glm::radians(-mouse_x), glm::vec3(0.0, 1.0f, 0.0f));
        glm::quat target_rotation = x_rotation * y_rotation;
        glm::quat new_rotation = glm::slerp(controller.prev_transform.rotation,
                                            target_rotation * glm::quat(
                                                glm::radians(glm::vec3(viewmodel.euler_offset))),
                                            dt * viewmodel.smoothing);
        glm::vec3 velocity = Physics::get_player_velocity(player.id);
        float y_vel = velocity.y;
        velocity.y = 0.0f;
        if (glm::length2(velocity) > 0.0f)
        {
            controller.time += dt;
        }
        else
        {
            controller.time = 0.0f;
        }

        glm::vec3 bob = glm::vec3(0.0f);
        bob.y += glm::sin(controller.time * viewmodel.frequency) * viewmodel.amplitude;
        bob.x += glm::cos(controller.time * viewmodel.frequency / 2.0f) * viewmodel.amplitude * 2.0f;
        bob.y += glm::clamp(-y_vel * viewmodel.vertical_velocity_multiplier,
                            -viewmodel.max_vertical_offset, viewmodel.max_vertical_offset);
        glm::vec3 new_position = glm::lerp(controller.prev_transform.position,
                                           bob + viewmodel.position_offset, dt * viewmodel.smoothing);
        controller.prev_transform.position = new_position;
        controller.prev_transform.rotation = new_rotation;

        glm::mat4 gun_mat = glm::mat4(1.0f);
        Entity camera = scene->get_entity_by_uuid(player.camera);
        auto &cam_transform = camera.get_transform();
        gun_mat = glm::translate(gun_mat, new_position);
        gun_mat *= glm::toMat4(new_rotation);
        gun_mat = glm::inverse(Renderer::get_camera_view(cam_transform)) * gun_mat;
        glm::quat orientation;
        glm::vec3 translation;
        glm::vec3 scale;
        glm::vec4 persp;
        glm::vec3 skew;
        glm::decompose(gun_mat, scale, orientation, translation, skew, persp);

        viewmodel_entity.get_transform().position = translation;
        viewmodel_entity.get_transform().rotation = orientation;
    }

    void PlayerControllerSystem::apply_friction(Scene *scene, entt::registry &registry, entt::entity entity, float t, float dt)
    {
        auto &controller = registry.get<PlayerControllerComponent>(entity);
        auto &player = registry.get<PlayerComponent>(entity);
        glm::vec3 v = controller.velocity;
        v.y = 0.0f;
        float speed = length(v);
        float drop = 0.0f;
        if (controller.grounded)
        {
            float control = speed < player.run_deceleration
                                ? player.run_deceleration
                                : speed;
            drop = control * player.friction * dt * t;
        }

        float new_speed = speed - drop;
        if (new_speed < 0.0f)
        {
            new_speed = 0.0f;
        }
        if (new_speed > 0.0f)
        {
            new_speed /= speed;
        }

        controller.velocity.x *= new_speed;
        controller.velocity.z *= new_speed;
    }

    void PlayerControllerSystem::acceleration(Scene *scene, entt::registry &registry, entt::entity entity, glm::vec3 goal_dir,
                                              float goal_speed, float accel, float dt)
    {
        auto &controller = registry.get<PlayerControllerComponent>(entity);
        float current_speed = glm::dot(controller.velocity, goal_dir);
        float add_speed = goal_speed - current_speed;
        if (add_speed <= 0)
        {
            return;
        }

        float accel_speed = accel * dt * goal_speed;
        if (accel_speed > add_speed)
        {
            accel_speed = add_speed;
        }
        controller.velocity.x += accel_speed * goal_dir.x;
        controller.velocity.z += accel_speed * goal_dir.z;
    }

    void PlayerControllerSystem::ground_move(Scene *scene,entt::registry &registry, entt::entity entity, glm::vec3 movement_input,
                                             float dt)
    {
        auto &controller = registry.get<PlayerControllerComponent>(entity);
        auto &player = registry.get<PlayerComponent>(entity);
        apply_friction(scene, registry, entity, !controller.jump_queued ? 1.0f : 0.0f, dt);
        if (length2(movement_input) != 0.0f)
        {
            movement_input = normalize(movement_input);
        }
        float goal_speed = length(movement_input) * player.move_speed;
        acceleration(scene, registry, entity, movement_input, goal_speed, player.run_acceleration, dt);
        controller.velocity.y = -player.gravity * dt;
        if (controller.jump_queued)
        {
            controller.velocity.y = player.jump_speed;
            controller.jump_queued = false;
        }
        else
        {
            //slope correct here
        }
    }

    void PlayerControllerSystem::air_move(Scene *scene, entt::registry &registry, entt::entity entity, glm::vec3 movement_input,
                                          bool strafing_only, float dt)
    {
        auto &controller = registry.get<PlayerControllerComponent>(entity);
        auto &player = registry.get<PlayerComponent>(entity);
        float accel;
        float wish_speed = glm::length(movement_input);
        wish_speed *= player.move_speed;
        if (length(movement_input) != 0.0f)
        {
            movement_input = normalize(movement_input);
        }

        float wish_speed2 = wish_speed;
        if (glm::dot(controller.velocity, movement_input) < 0)
        {
            accel = player.air_deceleration;
        }
        else
        {
            accel = player.air_acceleration;
        }

        if (strafing_only)
        {
            if (wish_speed > player.side_strafe_speed)
            {
                wish_speed = player.side_strafe_speed;
            }
            accel = player.side_strafe_acceleration;
        }
        acceleration(scene, registry, entity, movement_input, wish_speed, accel, dt);
        if (player.air_control > 0)
        {
            air_control(scene, registry, entity, movement_input, wish_speed2, !strafing_only, dt);
        }
        controller.velocity.y -= player.gravity * dt;
    }

    void PlayerControllerSystem::air_control(Scene *scene, entt::registry &registry, entt::entity entity, glm::vec3 movement_input,
                                             float target_speed, bool only_forward, float dt)
    {
        if (!only_forward || glm::abs(target_speed) < 0.0001f)
        {
            return;
        }

        auto &controller = registry.get<PlayerControllerComponent>(entity);
        float z_speed = controller.velocity.y;
        controller.velocity.y = 0.0f;

        float speed = glm::length(controller.velocity);
        if (speed != 0.0f)
        {
            controller.velocity = glm::normalize(controller.velocity);
        }

        float dot = glm::dot(controller.velocity, movement_input);
        float k = 32;
        k *= registry.get<PlayerComponent>(entity).air_control * dot * dot * dt;

        if (dot > 0)
        {
            controller.velocity *= speed * glm::length(movement_input) * k;
            controller.velocity = glm::normalize(controller.velocity);
        }
        controller.velocity.x *= speed;
        controller.velocity.y = z_speed;
        controller.velocity.z *= speed;
    }

    void PlayerControllerSystem::queue_jump(Scene *scene, entt::registry &registry, entt::entity entity)
    {
        auto &controller = registry.get<PlayerControllerComponent>(entity);
        if (Input::key_pressed(Input::Key::Space))
        {
            controller.jump_queued = true;
        }
        if (!Input::key_pressed(Input::Key::Space))
        {
            controller.jump_queued = false;
        }
    }

    void PlayerControllerSystem::update_gun(Scene *scene, entt::registry &registry, entt::entity entity, float dt)
    {
        auto &player = registry.get<PlayerComponent>(entity);
        auto &controller = registry.get<PlayerControllerComponent>(entity);
        if (controller.shot_timer < controller.gun_time)
        {
            controller.shot_timer += dt;
        }
        else
        {
            controller.is_firing = false;
            controller.is_reloading = false;
        }

        if (!controller.is_firing && !controller.is_reloading)
        {
            if (Input::mouse_pressed(Input::MouseButton::Left) && controller.current_ammo > 0)
            {
                LOG_INFO("Bang");
                Entity vm = scene->get_entity_by_uuid(player.viewmodel);
                auto &anim = vm.get_component<AnimatorComponent>();
                anim.play_one_shot_animation(AssetManager::get_animation_by_name("deagle_Rig|Rig|MK_Shot"));
                Audio::play_sound(controller.shoot_sound, 30);
                auto cam = scene->get_entity_by_uuid(player.camera);
                auto tr = cam.get_transform();
                scene->create_bullet(tr.position, tr.get_forward(), 25);
                controller.shot_timer = 0.0f;
                controller.gun_time = controller.rpm;
                controller.current_ammo--;
                controller.is_firing = true;
            }

            if (Input::key_pressed(Input::Key::R) && controller.current_ammo < controller.max_ammo)
            {
                LOG_INFO("RELOADING!");
                controller.shot_timer = 0.0f;
                controller.gun_time = controller.reload_time;
                Entity vm = scene->get_entity_by_uuid(player.viewmodel);
                auto &anim = vm.get_component<AnimatorComponent>();
                anim.play_one_shot_animation(AssetManager::get_animation_by_name("deagle_Rig|Rig|MK_ReloadFull"));
                Audio::play_sound(controller.reload_sound, 20);
                controller.is_reloading = true;
                controller.current_ammo = controller.max_ammo;
            }
        }

        std::string text = (std::string("Ammo ") + std::to_string(controller.current_ammo) + "/" + std::to_string(
                                controller.max_ammo));
        Engine::get_renderer()->draw_text(text.c_str(),
                                          glm::vec3(Engine::get_window()->get_width() - (text.length() * 48.0f),
                                                    660.0f, 0.0f), glm::vec4(1.0f), 0.6f);
        auto vel = controller.velocity;
        vel.y = 0.0f;
        float speed = glm::length(vel);
        std::string speed_text = std::string("Speed ") + std::to_string(speed);
        Engine::get_renderer()->draw_text(speed_text.c_str(),
                                          glm::vec3(Engine::get_window()->get_width() - (speed_text.length() * 48.0f),
                                                    690.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.6f);
    }
}
