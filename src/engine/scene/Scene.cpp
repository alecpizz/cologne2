//
// Created by alecpizz on 3/3/2025.
//

#include "Scene.h"

#include <engine/animation/AnimatorComponent.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/core/UUID.h>
#include <engine/physics/Physics.h>
#include <engine/renderer/types/Light.h>
#include <engine/util/FileUtil.h>
#include <engine/scripts/EditorCameraController.h>
#include <engine/scripts/PlayerController.h>
#include <engine/util/DebugScope.h>
#include <engine/util/Frustum.h>
#include <engine/util/Util.h>

#include "Components.h"
#include "Entity.h"
#include "SceneSaver.h"
#include "engine/physics/RaycastHitInfo.h"

namespace cologne
{
    Frustum cam_frustum;

    void Scene::initialize_special_types()
    {
        for (const auto entity: _registry.view<IDComponent>())
        {
            _entity_map[_registry.get<IDComponent>(entity).id] = entity;
        }
        auto view = _registry.view<TransformComponent, StaticColliderComponent>();
        for (auto entity: view)
        {
            Entity e = {entity, this};
            auto &transform = _registry.get<TransformComponent>(entity);
            auto &collider = _registry.get<StaticColliderComponent>(entity);
            if (collider.body_id > 0)
            {
                continue;
            }
            auto mesh = AssetManager::get_mesh_by_name(collider.mesh_name);
            if (!mesh)
            {
                continue;
            }
            uint32_t body_id = Physics::create_static_mesh_collider(
                e, transform, *mesh);
            collider.body_id = body_id;
        }

        for (const auto entity : _registry.view<PlayerComponent>())
        {
            auto& pc = _registry.get<PlayerComponent>(entity);
            PlayerCreateInfo info;
            info.position = _registry.get<TransformComponent>(entity).position;
            pc.id = Physics::create_player(info);
        }

        for (const auto entity: _registry.view<NativeScriptComponent>())
        {
            auto &ns = _registry.get<NativeScriptComponent>(entity);
            if (!ns.instance)
            {
                if (ns.type_name == entt::type_name<EditorCameraController>().value())
                {
                    ns.bind<EditorCameraController>();
                }
                else if (ns.type_name == entt::type_name<PlayerController>().value())
                {
                    ns.bind<PlayerController>();
                }
                else
                {
                    LOG_ERROR("NO TYPE FOUND FOR %s", ns.type_name.c_str());
                }
            }
        }

        for (const auto entity : _registry.view<AnimatorComponent>())
        {
            auto& ac = _registry.get<AnimatorComponent>(entity);
            if (ac.has_ragdoll())
            {
                ac.create_ragdoll({entity, this});
            }
        }
    }

    Scene::Scene()
    {
        DebugScope scope(__PRETTY_FUNCTION__);

        // create_static_model_entities("sponza2", {});
        //
        // // Entity glowCube = create_entity("glowing cube");
        // // glowCube.get_transform().position = glm::vec3(0.0f, 1.0f, 4.5f);
        // // glowCube.add_component<ModelComponent>(AssetManager::get_model_index_by_name("glowCube"), true);
        //
        // Entity man = create_entity("man");
        // man.get_transform().rotation =
        //         glm::quat(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f));
        // man.get_transform().position.y -= 0.25f;
        // auto enemy = man.add_component<EnemyComponent>();
        // Audio::add_sound(enemy.hurt_sound.c_str()); //todo: have asset manager do this
        // man.add_component<SkinnedModelComponent>("man");
        // AnimatorComponent &anim = man.add_component<AnimatorComponent>("man");
        // anim.play_base_animation(AssetManager::get_animation_by_name("man_Idle"));
        // anim.create_ragdoll(man);
        //
        // Entity man2 = create_entity("man2");
        // man2.get_transform().position = glm::vec3(0.0f, -0.25f, 2.5f);
        // man2.add_component<SkinnedModelComponent>("man");
        // AnimatorComponent &anim3 = man2.add_component<AnimatorComponent>("man");
        // anim3.play_base_animation(AssetManager::get_animation_by_name("man_Idle"));
        //
        // Entity revolver = create_entity("deagle");
        // revolver.add_component<SkinnedModelComponent>("deagle");
        // AnimatorComponent &anim2 = revolver.add_component<AnimatorComponent>("deagle");
        // anim2.play_base_animation(AssetManager::get_animation_by_name("deagle_Rig|Rig|MK_ReloadFull"));
        //
        // auto camera = create_entity("camera");
        // auto &c = camera.add_component<CameraComponent>();
        // c.primary = true;
        //
        // Entity viewModel = create_entity("viewmodel");
        // viewModel.add_component<SkinnedModelComponent>("vsk");
        // auto &anim4 = viewModel.add_component<AnimatorComponent>("vsk");
        // anim4.play_base_animation(AssetManager::get_animation_by_name("vsk_Idle"));
        // // viewModel.add_component<SkinnedModelComponent>(AssetManager::get_skinned_model_index_by_name("deagle"));
        // // viewModel.add_component<AnimatorComponent>(AssetManager::get_animation_index_by_name("deagle_Rig|Rig|MK_Idle"));
        // viewModel.add_component<ViewmodelComponent>();
        //
        // Entity player = create_entity("player");
        // //todo: REMOVE NATIVE SCRIPT COMPONENT!
        // auto &nc = player.add_component<NativeScriptComponent>();
        // nc.bind<PlayerController>();
        // nc.type_name = entt::type_name<PlayerController>::value();
        // PlayerCreateInfo info;
        // info.position = glm::vec3(-3.0f, 2.0f, 0.0f);
        // player.add_component<PlayerComponent>(Physics::create_player(info), camera.get_uuid(), viewModel.get_uuid());
        //
        // create_static_model_entities("Lantern", TransformComponent(
        //                                  glm::vec3(0.180f, -0.338f, -4.8f),
        //                                  glm::quat(glm::radians(glm::vec3(0.0f, -90.0f, 0.0f))),
        //                                  glm::vec3(1.0f)));
        //
        //
        // Entity spot_light = create_entity("spot light");
        // auto &lc = spot_light.add_component<LightComponent>();
        // lc.type = LightComponent::Spot;
        //
        //
        // Entity point_light = create_entity("point light");
        // auto &light2 = point_light.add_component<LightComponent>();
        // light2.color = glm::vec3(1, 0.7799999713897705, 0.5289999842643738);
        // light2.radius = 3.0f;
        // light2.strength = 6.0f;
        // light2.type = LightComponent::Point;
        // point_light.get_transform().position = glm::vec3(-6.0f, 5.0f, -5.0f);
        //
        // Entity point_light2 = create_entity("point light2");
        // auto &light3 = point_light2.add_component<LightComponent>();
        // light3 = light2;
        // light3.color = glm::vec3(0.2f, 0.9f, 0.15f);
        // point_light2.get_transform().position = glm::vec3(6.0f, 4.580, 3.796f);
        //
        // Entity point_light3 = create_entity("point light3");
        // auto &light4 = point_light3.add_component<LightComponent>();
        // light4.color = glm::vec3(0.078f, 0.656, 0.840f);
        // light4.radius = 3.13f;
        // light4.strength = 6.670f;
        // point_light3.get_transform().position = glm::vec3(-6.7, 1.4, -3.979);
        //
        // Entity lantern_light = create_entity("lantern light");
        // auto &light5 = lantern_light.add_component<LightComponent>();
        // light5.color = glm::vec3(1.0, 0.78, 0.529);
        // light5.radius = 3.670f;
        // lantern_light.get_transform().position = glm::vec3(0.243f, 1.073f, -3.608f);
        //
        // Entity evil_light = create_entity("evil light");
        // auto &light6 = evil_light.add_component<LightComponent>();
        // light6.radius = 3.670f;
        // light6.color = glm::vec3(1.0f, 0.0f, 0.0f);
        // light6.strength = 7.0f;
        // evil_light.get_transform().position = glm::vec3(-4.538, 1.082, 3.635);
        //
        // Entity hall_light = create_entity("hall light");
        // auto &light8 = hall_light.add_component<LightComponent>();
        // light8.radius = 3.780f;
        // light8.strength = 5.910f;
        // light8.color = glm::vec3(1.0f, 0.780f, 0.529f);
        // hall_light.get_transform().position = glm::vec3(2.795f, 0.419f, 3.638f);
        //
        // Entity dir_light = create_entity("directional light");
        // auto &light = dir_light.add_component<LightComponent>();
        // light.color = glm::vec3(1, 0.864, 0.709);
        // light.radius = 6.0f;
        // light.strength = 2.0f;
        // light.cast_shadows = true;
        // light.type = Directional;
        // dir_light.get_transform().position = glm::vec3(0.790f, 18.867f, 0.024f);
        // dir_light.get_transform().rotation =
        //         glm::quat(glm::radians(glm::vec3(88.500, 0.0f, 0.0f)));
        //
        //
        // Entity scene_camera = create_entity("Scene Camera");
        // auto &cam = scene_camera.add_component<CameraComponent>();
        // cam.primary = false;
        // auto &scn = scene_camera.add_component<NativeScriptComponent>();
        // scn.bind<EditorCameraController>();
        // scn.type_name = entt::type_name<EditorCameraController>().value();
        //
        //
        // create_static_model_entities("sofa", TransformComponent(glm::vec3(-.6f, -.5f, -2.270f),
        //                                                         glm::quat(glm::radians(glm::vec3(0.0f))),
        //                                                         glm::vec3(1.0f)));
        //

        SceneSaver saver_temp(this);
        // saver_temp.serialize(RESOURCES_PATH "scenes/test_scene.cscn");
        saver_temp.deserialize(RESOURCES_PATH "scenes/test_scene.cscn");
        initialize_special_types();
        Physics::create_infinite_ground_plane(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);
        re_calculate_bounds();
        LOG_INFO("Scene bounds are min (%f, %f, %f), max (%f, %f, %f)", _scene_bounds.min.x, _scene_bounds.min.y,
                 _scene_bounds.min.z, _scene_bounds.max.z, _scene_bounds.max.y, _scene_bounds.max.z);
        LOG_INFO("Scene size is (%f, %f, %f)", _scene_bounds.size().x, _scene_bounds.size().y, _scene_bounds.size().z);
        _particles.emplace_back(Particles());
        _particles[0].init(_scene_bounds, 20);
    }

    Scene::~Scene()
    {
        _particles.clear();
        _registry.clear();
    }


    void Scene::update(float delta_time)
    {
        //compute shaders. should do skinning here too :3
        if (!Engine::in_edit_mode())
        {
            for (auto &particle: Engine::get_scene()->get_particles())
            {
                particle.simulate();
            }
            auto animators = _registry.view<AnimatorComponent, ActiveComponent>();
            for (auto entity: animators)
            {
                if (!_registry.get<ActiveComponent>(entity).active)
                {
                    continue;
                }
                auto &animator = _registry.get<AnimatorComponent>(entity);
                animator.update(delta_time, _registry.get<WorldTransformComponent>(entity));
            }
        }

        //native scripting
        for (auto entity: _registry.view<NativeScriptComponent, ActiveComponent>())
        {
            auto &nsc = _registry.get<NativeScriptComponent>(entity);
            if (!nsc.instance)
            {
                nsc.instance = nsc.instantiate_script();
                nsc.instance->_entity = Entity{entity, this};
                nsc.instance->on_create();
            }

            if (!_registry.get<ActiveComponent>(entity).active)
            {
                continue;
            }
            if (Engine::in_edit_mode())
            {
                if (nsc.instance->get_runtime_mode() == RuntimeMode::EDITOR_ONLY
                    || nsc.instance->get_runtime_mode() == RuntimeMode::EDITOR_AND_GAME)
                {
                    nsc.instance->on_update(delta_time);
                }
            }
            else
            {
                if (nsc.instance->get_runtime_mode() == RuntimeMode::GAME_ONLY
                    || nsc.instance->get_runtime_mode() == RuntimeMode::EDITOR_AND_GAME)
                {
                    nsc.instance->on_update(delta_time);
                }
            }
        }

        if (!Engine::in_edit_mode())
        {
            auto bullets = _registry.view<BulletComponent>();
            RaycastHitInfo info;
            for (auto ent: bullets)
            {
                auto &bullet = _registry.get<BulletComponent>(ent);
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
                    }
                }
            }

            for (auto entt: bullets)
            {
                destroy_entity({entt, this});
            }
        }

        auto camera = !Engine::in_edit_mode() ? get_primary_camera() : get_scene_camera();
        auto tr = camera.get_transform();
        auto cm = camera.get_component<CameraComponent>();
        Engine::get_renderer()->submit_camera_transform(tr, cm);
        cam_frustum.update(Renderer::get_camera_projection(tr, cm) * Renderer::get_camera_view(tr));

        for (auto entity: _registry.view<LightComponent, WorldTransformComponent, ActiveComponent>())
        {
            auto [light, transform, active] =
                    _registry.get<LightComponent, WorldTransformComponent, ActiveComponent>(entity);
            if (!active)
            {
                continue;
            }
            // //TEMP, need to figure out a better radius culling tech
            // if (light.radius < 6.0f && !cam_frustum.intersect_point(transform.position))
            // {
            //     continue;
            // }
            Engine::get_renderer()->submit_light(Light(light, TransformComponent(transform)));
        }

        glm::vec3 ray_start = tr.position, ray_dir = tr.get_forward();
        RaycastHitInfo info;
        if (!Engine::in_edit_mode())
        {
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
                }
                Engine::get_renderer()->draw_line(info.hit_point, info.hit_point + info.hit_normal * 0.1f,
                                                  glm::max(info.hit_normal, glm::vec3(0.1f, 0.1f, 0.1f)));
            }
        }


        update_transforms();

        if (Engine::in_edit_mode())
        {
            //temp?
            auto view = _registry.view<StaticColliderComponent, WorldTransformComponent>();
            for (auto entity: view)
            {
                Entity e = {entity, this};
                if (e.get_component<ActiveComponent>().active)
                {
                    if (!e.get_component<StaticColliderComponent>().body_enabled)
                    {
                        Physics::enable_body(e.get_component<StaticColliderComponent>().body_id);
                        e.get_component<StaticColliderComponent>().body_enabled = true;
                    }
                    Physics::sync_transform(e);
                }
                else
                {
                    if (e.get_component<StaticColliderComponent>().body_enabled)
                    {
                        Physics::disable_body(e.get_component<StaticColliderComponent>().body_id);
                        e.get_component<StaticColliderComponent>().body_enabled = false;
                    }
                }
            }
        }
        //submit draw calls
        auto view = _registry.view<ModelComponent, WorldTransformComponent, ActiveComponent>();
        for (auto entity: view)
        {
            auto [m, tr, active] =
                    view.get<ModelComponent, WorldTransformComponent, ActiveComponent>(entity);
            if (!active)
            {
                continue;
            }
            Model *model = AssetManager::get_model_by_name(m.model_name);
            for (int32_t idx: model->get_mesh_indices())
            {
                Engine::get_renderer()->submit_render_item(
                    RenderItem(idx, tr, m.gi_only, static_cast<uint32_t>(entity)));
            }
        }

        auto view2 = _registry.view<MeshComponent, WorldTransformComponent, ActiveComponent>();
        for (auto entity: view2)
        {
            auto [m, tr, active] = view2.get<MeshComponent, WorldTransformComponent, ActiveComponent>(entity);
            if (!active)
            {
                continue;
            }
            Engine::get_renderer()->
                    submit_render_item(RenderItem(AssetManager::get_mesh_index_by_name(m.mesh_name), tr, false,
                                                  static_cast<uint32_t>(entity)));
        }

        auto view3 = _registry.view<SkinnedModelComponent, WorldTransformComponent, ActiveComponent>();
        for (auto entity: view3)
        {
            auto [m, tr, active] =
                    view3.get<SkinnedModelComponent, WorldTransformComponent, ActiveComponent>(entity);
            //culling step would be here probably? though for GI i dunno. might have to pack into render item
            if (!active)
            {
                continue;
            }
            SkinnedModel *skinned_model = AssetManager::get_skinned_model_by_name(m.model_name);
            std::vector<glm::mat4> bones;
            if (_registry.all_of<AnimatorComponent>(entity))
            {
                auto &animator = _registry.get<AnimatorComponent>(entity);
                bones = animator.get_skinning_matrices();
            }
            for (int32_t mesh_index: skinned_model->get_mesh_indices())
            {
                SkinnedRenderItem item;
                item.mesh_idx = mesh_index;
                item.transform = tr;
                item.entity_id = static_cast<uint32_t>(entity);
                item.bones = bones;
                Engine::get_renderer()->submit_skinned_render_item(item);
            }
        }
    }

    AABB Scene::re_calculate_bounds()
    {
        _scene_bounds = {};
        auto view = _registry.view<TransformComponent, ModelComponent>();
        for (const auto entity: view)
        {
            auto [tr, m] = view.get<TransformComponent, ModelComponent>(entity);
            const auto model = AssetManager::get_model_by_name(m.model_name);
            AABB aabb = model->get_aabb();
            aabb.min *= glm::vec4(tr.scale, 1.0);
            aabb.max *= glm::vec4(tr.scale, 1.0);
            _scene_bounds.expand(aabb.min);
            _scene_bounds.expand(aabb.max);
        }
        auto view2 = _registry.view<TransformComponent, MeshComponent>();
        for (const auto entity: view2)
        {
            auto [tr, m] = view2.get<TransformComponent, MeshComponent>(entity);
            const auto mesh = AssetManager::get_mesh_by_name(m.mesh_name);
            AABB aabb = mesh->get_aabb();
            aabb.min *= glm::vec4(tr.scale, 1.0);
            aabb.max *= glm::vec4(tr.scale, 1.0);
            aabb.min += glm::vec4(tr.position, 1.0);
            aabb.max += glm::vec4(tr.position, 1.0);
            _scene_bounds.expand(aabb.min);
            _scene_bounds.expand(aabb.max);
        }
        return _scene_bounds;
    }

    AABB Scene::get_bounds() const
    {
        return _scene_bounds;
    }

    std::vector<Particles> &Scene::get_particles()
    {
        return _particles;
    }

    Entity Scene::create_entity_with_uuid(UUID id, const std::string &name)
    {
        Entity entity = {_registry.create(), this};
        entity.add_component<IDComponent>(id);
        entity.add_component<TransformComponent>();
        entity.add_component<TagComponent>(name.empty() ? "Entity" : name);
        entity.add_component<ActiveComponent>(true);
        entity.add_component<WorldTransformComponent>();
        _entity_map[id] = entity;
        return entity;
    }


    Entity Scene::create_entity(const std::string &name)
    {
        return create_entity_with_uuid(UUID(), name);
    }

    Entity Scene::get_entity_by_uuid(UUID uuid)
    {
        if (!_entity_map.contains(uuid))
        {
            return {};
        }
        return {_entity_map.at(uuid), this};
    }

    Entity Scene::create_static_model_entities(const char *model_name, const TransformComponent &parent_transform,
                                               bool create_colliders)
    {
        auto model = AssetManager::get_model_by_name(model_name);
        if (!model)
        {
            return {};
        }
        Entity parent = create_entity(std::string(model_name));
        parent.get_transform() = parent_transform;
        parent.get_component<WorldTransformComponent>().transform = parent_transform.get_mat4();
        if (model->get_mesh_indices().size() == 1)
        {
            parent.add_component<MeshComponent>(model->get_mesh_indices()[0]);
            auto &col = parent.add_component<StaticColliderComponent>();
            auto mesh_by_index = AssetManager::get_mesh_by_index(model->get_mesh_indices()[0]);
            col.mesh_name = mesh_by_index->get_name();
            if (create_colliders)
            {
                auto mesh = AssetManager::get_mesh_by_index(model->get_mesh_indices()[0]);
                uint32_t body_id = Physics::create_static_mesh_collider(
                    parent, parent_transform, *mesh);
                col.body_id = body_id;
            }
            return parent;
        }
        auto &parent_comp = parent.add_component<ParentComponent>();
        for (auto idx: model->get_mesh_indices())
        {
            const auto mesh = AssetManager::get_mesh_by_index(idx);
            Entity sub_mesh = create_entity(mesh->get_name());
            sub_mesh.add_component<ChildComponent>(parent.get_uuid());
            parent_comp.children.emplace_back(sub_mesh.get_uuid());
            sub_mesh.add_component<MeshComponent>(idx);
            sub_mesh.get_transform() = TransformComponent(mesh->get_inverse_bind_pose());
            sub_mesh.get_component<WorldTransformComponent>().transform =
                    parent_transform.get_mat4() * mesh->get_inverse_bind_pose();
            auto &col = sub_mesh.add_component<StaticColliderComponent>();
            col.mesh_name = mesh->get_name();
            if (create_colliders)
            {
                TransformComponent temp = TransformComponent(
                    parent_transform.get_mat4() * mesh->get_inverse_bind_pose());
                uint32_t body_id = Physics::create_static_mesh_collider(
                    sub_mesh, temp, *mesh);
                col.body_id = body_id;
            }
            update_transforms();
        }
        return parent;
    }


    void Scene::destroy_entity(Entity entity)
    {
        _entity_map.erase(entity.get_uuid());
        if (entity.has_component<ParentComponent>())
        {
            for (auto e: entity.get_component<ParentComponent>().children)
            {
                destroy_entity(get_entity_by_uuid(e));
            }
        }
        if (entity.has_component<StaticColliderComponent>())
        {
            Physics::destroy_body(entity.get_component<StaticColliderComponent>().body_id);
        }
        _registry.destroy(entity);
    }

    Entity Scene::get_primary_camera()
    {
        for (auto entity: _registry.view<CameraComponent>())
        {
            Entity e = {entity, this};
            if (e.get_component<CameraComponent>().primary)
            {
                return e;
            }
        }
        return {};
    }

    Entity Scene::get_scene_camera()
    {
        for (auto entity: _registry.view<CameraComponent>())
        {
            Entity e = {entity, this};
            if (!e.get_component<CameraComponent>().primary)
            {
                return e;
            }
        }
        return {};
    }

    void Scene::copy_scene_camera_to_primary_camera()
    {
        Entity scene_cam = get_scene_camera();
        Entity game_cam = get_primary_camera();
        if (!scene_cam || !game_cam) return;
        scene_cam.get_transform().position = game_cam.get_transform().position;
    }

    void Scene::update_transforms()
    {
        for (auto entity: _registry.view<WorldTransformComponent, TransformComponent>())
        {
            auto &tr = _registry.get<TransformComponent>(entity);
            auto &wd = _registry.get<WorldTransformComponent>(entity);
            wd.transform = tr.get_mat4();
        }

        auto parent_view = _registry.view<ParentComponent>();
        for (auto entity: parent_view)
        {
            update_children(entity);
        }
    }

    void Scene::create_bullet(glm::vec3 pos, glm::vec3 dir, float damage)
    {
        static int bullet_count = 0;
        Entity e = create_entity("bullet" + bullet_count++);
        e.add_component<BulletComponent>(pos, dir, damage);
    }

    void Scene::update_children(entt::entity parent)
    {
        auto &parent_wld = _registry.get<WorldTransformComponent>(parent);
        auto &parent_comp = _registry.get<ParentComponent>(parent);

        for (auto child_id: parent_comp.children)
        {
            Entity child = get_entity_by_uuid(child_id);
            if (_registry.valid(child))
            {
                auto &child_transform = _registry.get<TransformComponent>(child);
                auto &child_world_transform = _registry.get<WorldTransformComponent>(child);
                child_world_transform.transform = parent_wld.transform * child_transform.get_mat4();
                if (_registry.any_of<ParentComponent>(child))
                {
                    update_children(child);
                }
            }
        }
    }
}
