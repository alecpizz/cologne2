//
// Created by alecpizz on 3/3/2025.
//

#include "Scene.h"

#include <engine/animation/Animation.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/physics/Physics.h>
#include <engine/util/FileUtil.h>
#include <engine/scripts/EditorCameraController.h>
#include <engine/scripts/PlayerController.h>
#include <engine/util/DebugScope.h>
#include <engine/util/Frustum.h>

#include "Components.h"
#include "Entity.h"
#include "engine/physics/RaycastHitInfo.h"

namespace cologne
{
    Frustum cam_frustum;

    Scene::Scene()
    {
        DebugScope scope(__PRETTY_FUNCTION__);
        //Create entities
        Entity sponza = create_entity("sponza");
        sponza.get_component<TransformComponent>().scale = glm::vec3(0.01f);
        sponza.add_component<ModelComponent>(AssetManager::get_model_index_by_name("sponza2"), false);

        auto model = AssetManager::get_model_by_name("sponza2");
        auto tr = sponza.get_component<TransformComponent>();
        for (auto &mesh: model->get_meshes())
        {
            Entity collider = create_entity(mesh.get_name() + "_Collider");
            collider.get_component<TransformComponent>() = tr;
            uint32_t body_id = Physics::create_static_mesh_collider(collider, tr, mesh.get_vertices(),
                                                                    mesh.get_indices());
            collider.add_component<StaticColliderComponent>(body_id);
        }

        // Entity glowCube = create_entity("glowing cube");
        // glowCube.get_component<TransformComponent>().position = glm::vec3(0.0f, 1.0f, 4.5f);
        // glowCube.add_component<ModelComponent>(AssetManager::get_model_index_by_name("glowCube"), true);

        Entity man = create_entity("man");
        man.add_component<SkinnedModelComponent>(AssetManager::get_skinned_model_index_by_name("man"));
        man.add_component<AnimatorComponent>(AssetManager::get_animation_index_by_name("man_Idle"));

        Entity revolver = create_entity("deagle");
        revolver.add_component<SkinnedModelComponent>(AssetManager::get_skinned_model_index_by_name("deagle"));
        revolver.add_component<AnimatorComponent>(AssetManager::get_animation_index_by_name("deagle_Rig|Rig|MK_ReloadFull"));

        Entity vsk = create_entity("vsk");
        vsk.add_component<SkinnedModelComponent>(AssetManager::get_skinned_model_index_by_name("vsk"));
        vsk.add_component<AnimatorComponent>(AssetManager::get_animation_index_by_name("vsk_Fire"));
        vsk.get_component<TransformComponent>().position = glm::vec3(0.0f, 0.3f, 1.0f);

        auto camera = create_entity("camera");
        auto &c = camera.add_component<CameraComponent>();
        c.primary = true;

        Entity viewModel = create_entity("viewmodel");
        viewModel.add_component<SkinnedModelComponent>(AssetManager::get_skinned_model_index_by_name("deagle"));
        viewModel.add_component<AnimatorComponent>(AssetManager::get_animation_index_by_name("deagle_Rig|Rig|MK_Idle"));
        viewModel.add_component<ViewmodelComponent>();

        Entity player = create_entity("player");
        player.add_component<NativeScriptComponent>().bind<PlayerController>();
        PlayerCreateInfo info;
        info.position = glm::vec3(-3.0f, 2.0f, 0.0f);
        player.add_component<PlayerComponent>(Physics::create_player(info), camera, viewModel);

        Entity lamp = create_entity("lamp");
        lamp.add_component<ModelComponent>(AssetManager::get_model_index_by_name("Lantern"));
        lamp.get_component<TransformComponent>().position = glm::vec3(0.180f, 0.0f, -4.8f);
        lamp.get_component<TransformComponent>().rotation = glm::quat(glm::radians(glm::vec3(0.0f, -90.0f, 0.0f)));
        model = AssetManager::get_model_by_name("Lantern");
        tr = lamp.get_component<TransformComponent>();
        for (auto &mesh: model->get_meshes())
        {
            Entity collider = create_entity(mesh.get_name());
            collider.get_component<TransformComponent>() = tr;
            uint32_t body_id = Physics::create_static_mesh_collider(collider, tr, mesh.get_vertices(),
                                                                    mesh.get_indices());
            collider.add_component<StaticColliderComponent>(body_id);
        }

        Entity dir_light = create_entity("directional light");
        auto &light = dir_light.add_component<LightComponent>();
        light.color = glm::vec3(1, 0.864, 0.709);
        light.radius = 6.0f;
        light.strength = 2.0f;
        light.type = 0;
        dir_light.get_component<TransformComponent>().position = glm::vec3(0.790f, 18.867f, 0.024f);
        dir_light.get_component<TransformComponent>().rotation =
                glm::quat(glm::radians(glm::vec3(82.300, 0.0f, 0.0f)));


        Entity point_light = create_entity("point light");
        auto &light2 = point_light.add_component<LightComponent>();
        light2.color = glm::vec3(1, 0.7799999713897705, 0.5289999842643738);
        light2.radius = 3.0f;
        light2.strength = 6.0f;
        light2.type = LightComponent::Point;
        point_light.get_component<TransformComponent>().position = glm::vec3(-6.0f, 5.0f, -5.0f);

        Entity point_light2 = create_entity("point light2");
        auto &light3 = point_light2.add_component<LightComponent>();
        light3 = light2;
        light3.color = glm::vec3(0.2f, 0.9f, 0.15f);
        point_light2.get_component<TransformComponent>().position = glm::vec3(6.0f, 3.4, 5.0f);

        Entity scene_camera = create_entity("Scene Camera");
        auto &cam = scene_camera.add_component<CameraComponent>();
        cam.primary = false;
        scene_camera.add_component<NativeScriptComponent>().bind<EditorCameraController>();


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
        for (auto &particle: Engine::get_scene()->get_particles())
        {
            particle.simulate();
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

        auto animators = _registry.view<AnimatorComponent>();
        for (auto entity: animators)
        {
            auto &animator = _registry.get<AnimatorComponent>(entity);
            animator.update_animation(delta_time);
        }

        auto camera = !Engine::in_edit_mode() ? get_primary_camera() : get_scene_camera();
        auto tr = camera.get_component<TransformComponent>();
        auto cm = camera.get_component<CameraComponent>();
        Engine::get_renderer()->submit_camera_transform(tr, cm);
        cam_frustum.update(Renderer::get_camera_projection(tr, cm) * Renderer::get_camera_view(tr));

        for (auto entity: _registry.view<LightComponent, TransformComponent, ActiveComponent>())
        {
            auto [light, transform, active] = _registry.get<LightComponent, TransformComponent,
                ActiveComponent>(entity);
            if (!active)
            {
                continue;
            }
            // //TEMP, need to figure out a better radius culling tech
            // if (light.radius < 6.0f && !cam_frustum.intersect_point(transform.position))
            // {
            //     continue;
            // }
            Engine::get_renderer()->submit_light(Light(light, transform));
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
                static int light_count = 0;
                Engine::get_renderer()->draw_line(info.hit_point, info.hit_point + info.hit_normal * 0.1f,
                                                  glm::max(info.hit_normal, glm::vec3(0.1f, 0.1f, 0.1f)));
                if (Input::mouse_pressed(Input::MouseButton::Left))
                {
                    Entity light = create_entity("Point Light" + std::to_string(light_count++));
                    light.get_component<TransformComponent>().position = info.hit_point + info.hit_normal * 0.5f;
                    auto &lc = light.add_component<LightComponent>();
                    lc.color = glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f));
                }
            }
        }

        //submit draw calls
        auto view = _registry.view<ModelComponent, TransformComponent, ActiveComponent>();
        for (auto entity: view)
        {
            auto [m, tr, active] =
                    view.get<ModelComponent, TransformComponent, ActiveComponent>(entity);
            //culling step would be here probably? though for GI i dunno. might have to pack into render item
            if (!active)
            {
                continue;
            }
            Model *model = AssetManager::get_model_by_index(m.id);
            Engine::get_renderer()->submit_render_item(RenderItem(model, tr, m.gi_only, static_cast<uint32_t>(entity)));
        }

        auto view2 = _registry.view<SkinnedModelComponent, TransformComponent, ActiveComponent>();
        for (auto entity: view2)
        {
            auto [m, tr, active] =
                    view2.get<SkinnedModelComponent, TransformComponent, ActiveComponent>(entity);
            //culling step would be here probably? though for GI i dunno. might have to pack into render item
            if (!active)
            {
                continue;
            }
            SkinnedRenderItem item;
            if (_registry.all_of<AnimatorComponent>(entity))
            {
                auto &animator = _registry.get<AnimatorComponent>(entity);
                item.bones = animator.get_bones();
            }
            SkinnedModel *skinned_model = AssetManager::get_skinned_model_by_index(m.id);
            item.skinned_model = skinned_model;
            item.transform = tr;
            item.id = static_cast<uint32_t>(entity);
            Engine::get_renderer()->submit_skinned_render_item(item);
        }
    }

    AABB Scene::re_calculate_bounds()
    {
        _scene_bounds = {};
        auto view = _registry.view<TransformComponent, ModelComponent>();
        for (const auto entity: view)
        {
            auto [tr, m] = view.get<TransformComponent, ModelComponent>(entity);
            const auto model = AssetManager::get_model_by_index(m.id);
            AABB aabb = model->get_aabb();
            aabb.min *= tr.scale;
            aabb.max *= tr.scale;
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

    Entity Scene::create_entity(const std::string &name)
    {
        //all entities have a transform, a tag/name, and a bool for whether or not they are currently active
        Entity entity = {_registry.create(), this};
        entity.add_component<TransformComponent>();
        entity.add_component<TagComponent>(name.empty() ? "Entity" : name);
        entity.add_component<ActiveComponent>(true);
        return entity;
    }

    void Scene::destroy_entity(Entity entity)
    {
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
        scene_cam.get_component<TransformComponent>().position = game_cam.get_component<TransformComponent>().position;
    }
}
