//
// Created by alecpizz on 3/3/2025.
//

#include "Scene.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/core/UUID.h>
#include <engine/navigation/Navigation.h>
#include <engine/physics/Physics.h>
#include <engine/physics/RaycastHitInfo.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Light.h>
#include <engine/util/DebugScope.h>

#include "Components/Components.h"
#include "Components/ComponentRegistry.h"
#include "Entity.h"
#include "SceneSaver.h"
#include "systems/AnimationSystem.h"
#include "systems/BloodSystem.h"
#include "systems/BulletSystem.h"
#include "systems/EditorCameraControllerSystem.h"
#include "systems/InteractionSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/PlayerControllerSystem.h"
#include "systems/RagdollSystem.h"
#include "systems/RendererSystem.h"
#include "systems/System.h"
#include "systems/TransformSystem.h"
#include "systems/NPCSystem.h"

namespace cologne
{
    static std::vector<std::unique_ptr<System> > systems;

    void Scene::setup_blank_scene()
    {
        Entity plane = create_static_model_entities("plane", {});
        plane.get_transform().scale = glm::vec3(10.0f, 1.0f, 10.0f);

        Entity dir_light = create_entity("directional light");
        auto &light = dir_light.add_component<LightComponent>();
        light.color = glm::vec3(1, 0.864, 0.709);
        light.radius = 6.0f;
        light.strength = 2.0f;
        light.cast_shadows = true;
        light.type = Directional;
        dir_light.get_transform().position = glm::vec3(0.790f, 18.867f, 0.024f);
        dir_light.get_transform().rotation =
                glm::quat(glm::radians(glm::vec3(88.500, 0.0f, 0.0f)));
        Physics::create_infinite_ground_plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
        create_player_entity(glm::vec3(-3.0f, 2.0f, 0.0f));
        create_scene_camera_entity();
        _particles.emplace_back(Particles());
        for (const auto entity: _registry.view<IDComponent>())
        {
            _entity_map[_registry.get<IDComponent>(entity).id] = entity;
        }
    }

    Scene::Scene()
    {
        DebugScope scope(__PRETTY_FUNCTION__);
        re_calculate_bounds();
        for (const auto entity: _registry.view<IDComponent>())
        {
            _entity_map[_registry.get<IDComponent>(entity).id] = entity;
        }
    }

    Scene::Scene(const char *path)
    {
        if (std::string(path).empty())
        {
            LOG_ERROR("NO PATH TO SCENE");
            return;
        }
        SceneSaver saver_temp(this);
        saver_temp.deserialize(path);
        Physics::create_infinite_ground_plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
        re_calculate_bounds();
        LOG_INFO("Scene bounds are min (%f, %f, %f), max (%f, %f, %f)", _scene_bounds.min.x, _scene_bounds.min.y,
                 _scene_bounds.min.z, _scene_bounds.max.z, _scene_bounds.max.y, _scene_bounds.max.z);
        LOG_INFO("Scene size is (%f, %f, %f)", _scene_bounds.size().x, _scene_bounds.size().y, _scene_bounds.size().z);
        _particles.emplace_back(Particles());
        _particles[0].init(_scene_bounds, 5);
        for (const auto entity: _registry.view<IDComponent>())
        {
            _entity_map[_registry.get<IDComponent>(entity).id] = entity;
        }
    }

    Scene::~Scene()
    {
        LOG_INFO("Cleaning Up Scene");
        _particles.clear();
        _registry.clear();
    }


    void Scene::update_runtime(float delta_time)
    {
        assert(_scene_name == "RUNTIME_SCENE");
        for (const auto &system: systems)
        {
            if (system->get_update_flags() & RUNTIME)
            {
                system->on_update(this, delta_time);
            }
        }

        for (auto &particle: Engine::get_scene()->get_particles())
        {
            particle.simulate();
        }


        auto camera = get_primary_camera();
        auto tr = camera.get_transform();
        auto cm = camera.get_component<CameraComponent>();
        Engine::get_renderer()->submit_camera_transform(tr, cm);
    }

    void Scene::update_editor(float delta_time)
    {
        for (const auto &system: systems)
        {
            if (system->get_update_flags() & EDITOR)
            {
                system->on_update(this, delta_time);
            }
        }
        auto camera = get_scene_camera();
        auto tr = camera.get_transform();
        auto cm = camera.get_component<CameraComponent>();
        Engine::get_renderer()->submit_camera_transform(tr, cm);
    }

    void Scene::on_enter_play_mode()
    {
        setup_entity_map();
        re_calculate_bounds();
        for (const auto &system: systems)
        {
            if (system->get_update_flags() & RUNTIME)
            {
                system->on_scene_start(this);
            }
        }
    }

    void Scene::on_exit_play_mode()
    {
        Physics::delete_all_bodies();
        Engine::get_renderer()->clear_lights();
    }

    void Scene::setup_entity_map()
    {
        _entity_map.clear();
        for (const auto entity: _registry.view<IDComponent>())
        {
            _entity_map[_registry.get<IDComponent>(entity).id] = entity;
        }
    }

    void Scene::on_enter_edit_mode()
    {
        Physics::delete_all_bodies();
        Engine::get_renderer()->clear_lights();
        setup_entity_map();
        re_calculate_bounds();
        //TEMP, bad org
        Navigation::cleanup();
        Navigation::init_navmesh(this);
    }

    void Scene::on_exit_edit_mode()
    {
        Engine::get_renderer()->clear_lights();
        Physics::delete_all_bodies();
    }

    Ref<Scene> Scene::copy(Ref<Scene> scene)
    {
        Ref<Scene> result = create_ref<Scene>();
        result->_scene_name = "RUNTIME_SCENE";
        auto &src_registry = scene->get_raw_registry();
        auto &dest_registry = result->get_raw_registry();

        src_registry.view<entt::entity>().each([&](entt::entity entity)
        {
            if (!src_registry.valid(entity))
            {
                LOG_WARN("bad entity");
            }
            if (entity == entt::null)
            {
                LOG_WARN("NULL ENTITY");
                return;
            }
            auto dest_entity = dest_registry.create();
            assert(dest_entity != entt::null);
        });
        for (auto [id, storage]: src_registry.storage())
        {
            using namespace entt::literals;
            if (!ComponentRegistry::get_component_map().contains(storage.type().hash()))
            {
                continue;
            }

            entt::resolve(storage.type()).invoke("copy"_hs, {}, entt::forward_as_meta(storage),
                                                 entt::forward_as_meta(dest_registry));
            LOG_INFO("Copied %s", ComponentRegistry::get_component_map().at(storage.type().hash()).c_str());
        }

        return result;
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
            aabb = aabb.transform_by_mat4(tr.get_mat4());
            _scene_bounds.expand(aabb.min);
            _scene_bounds.expand(aabb.max);
        }
        auto view2 = _registry.view<TransformComponent, MeshComponent>();
        for (const auto entity: view2)
        {
            auto [tr, m] = view2.get<TransformComponent, MeshComponent>(entity);
            const auto mesh = AssetManager::get_mesh_by_name(m.mesh_name);
            AABB aabb = mesh->get_aabb();
            aabb = aabb.transform_by_mat4(tr.get_mat4());
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
            re_calculate_bounds();
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
        }
        re_calculate_bounds();
        return parent;
    }

    Entity Scene::create_player_entity(glm::vec3 pos)
    {
        auto camera = create_entity("camera");
        auto &c = camera.add_component<CameraComponent>();
        c.primary = true;

        Entity viewModel = create_entity("viewmodel");
        viewModel.add_component<SkinnedModelComponent>("deagle");
        viewModel.add_component<AnimatorComponent>();
        viewModel.add_component<ViewmodelComponent>();

        Entity player = create_entity("player");
        PlayerCreateInfo info;
        info.position = glm::vec3(pos);
        player.add_component<PlayerComponent>(Physics::create_player(info), camera.get_uuid(), viewModel.get_uuid());
        return player;
    }


    Entity Scene::create_scene_camera_entity()
    {
        Entity scene_camera = create_entity("Scene Camera");
        _registry.emplace<HideInEditorComponent>(scene_camera);
        auto &cam = scene_camera.add_component<CameraComponent>();
        cam.primary = false;
        _registry.emplace_or_replace<EditorCameraComponent>(scene_camera);
        return scene_camera;
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
        _registry.destroy(entity);
    }

    void Scene::destroy_entity(uint32_t id)
    {
        destroy_entity(Entity(static_cast<entt::entity>(id), this));
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

    void Scene::duplicate_recursive(Entity source, std::unordered_map<UUID, UUID> &old_to_new_map)
    {
        const auto destination = _registry.create();

        for (auto &&[id, storage]: _registry.storage())
        {
            if (storage.contains(source))
            {
                storage.push(destination, storage.value(source));
            }
        }


        Entity new_entity = {destination, this};
        auto &id_comp = new_entity.get_component<IDComponent>();
        UUID old_uuid = id_comp.id;
        id_comp.id = UUID();
        old_to_new_map[old_uuid] = id_comp.id;
        _entity_map[id_comp.id] = new_entity;

        if (new_entity.has_component<StaticColliderComponent>())
        {
            auto &collider = new_entity.get_component<StaticColliderComponent>();
            auto mesh = AssetManager::get_mesh_by_name(collider.mesh_name);
            if (mesh)
            {
                uint32_t body_id = Physics::create_static_mesh_collider(
                    new_entity, new_entity.get_transform(), *mesh);
                collider.body_id = body_id;
            }
        }

        if (new_entity.has_component<ParentComponent>())
        {
            auto &source_parent_component = source.get_component<ParentComponent>();
            for (auto &child_uuid: source_parent_component.children)
            {
                Entity child_entity = get_entity_by_uuid(child_uuid);
                if (child_entity)
                {
                    duplicate_recursive(child_entity, old_to_new_map);
                }
            }
        }
    }

    Entity Scene::duplicate_entity(Entity source)
    {
        if (!source)
        {
            LOG_ERROR("NO SOURCE ENTITY");
            return {};
        }

        std::unordered_map<UUID, UUID> old_to_new_map;
        duplicate_recursive(source, old_to_new_map);

        for (const auto &[old_id, new_id]: old_to_new_map)
        {
            Entity new_entity = get_entity_by_uuid(new_id);
            if (new_entity.has_component<ChildComponent>())
            {
                auto &child_comp = new_entity.get_component<ChildComponent>();
                UUID old_parent_uuid = child_comp.parent;
                child_comp.parent = old_to_new_map.at(old_parent_uuid);
            }

            if (new_entity.has_component<ParentComponent>())
            {
                auto &parent_comp = new_entity.get_component<ParentComponent>();

                std::vector<UUID> new_children_uuids;
                new_children_uuids.reserve(parent_comp.children.size());
                for (const auto &old_child_uuid: parent_comp.children)
                {
                    new_children_uuids.push_back(old_to_new_map.at(old_child_uuid));
                }
                parent_comp.children = new_children_uuids;
            }
        }
        UUID top_level = old_to_new_map.at(source.get_uuid());
        return get_entity_by_uuid(top_level);
    }

    void Scene::copy_scene_camera_to_primary_camera()
    {
        Entity scene_cam = get_scene_camera();
        Entity game_cam = get_primary_camera();
        if (!scene_cam || !game_cam) return;
        scene_cam.get_transform().position = game_cam.get_transform().position;
    }

    void Scene::create_bullet(glm::vec3 pos, glm::vec3 dir, float damage)
    {
        static int bullet_count = 0;
        Entity e = create_entity("bullet" + bullet_count++);
        e.add_component<BulletComponent>(pos, dir, damage);
    }

    void Scene::spawn_blood(glm::vec3 pos, glm::vec3 dir)
    {
        static int blood_count = 0;
        Entity vat_blood = create_entity("vat blood" + blood_count++);
        glm::vec3 world_up = glm::abs(glm::dot(dir, glm::vec3(0, 1, 0))) > 0.99f
                                 ? glm::vec3(1, 0, 0)
                                 : glm::vec3(0, 1, 0);
        glm::vec3 right = glm::normalize(glm::cross(world_up, dir));
        glm::vec3 up = glm::cross(dir, right);

        glm::mat4 rotation = glm::mat4(glm::mat3(right, up, dir));
        glm::mat4 rotation_90 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
        glm::vec3 scale = glm::vec3(12.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model *= rotation;
        model *= rotation_90;
        model = glm::scale(model, scale);
        model *= glm::translate(glm::mat4(1.0f), glm::vec3(-0.08f, -0.23f, -0.155f));
        vat_blood.get_transform() = model;

        auto &blood = vat_blood.add_component<BloodSplatterComponent>();
        blood.mesh_name = "blood_mesh_TRIANGLE_CLOUD";
        blood.position_texture_name = "blood_pos";
        blood.normal_texture_name = "blood_norm";
        blood.time = 0.0f;

        RaycastHitInfo hit_info;
        if (Physics::raycast(pos, glm::vec3(0.0f, -1.0f, 0.0f), 100.0f, Physics::NON_MOVING, hit_info))
        {
            Entity decal_blood = create_entity("decal blood" + blood_count);
            decal_blood.get_transform().position = hit_info.hit_point;
            glm::vec3 rot = glm::vec3(0.0f);
            rot.y = glm::linearRand(0.0f, glm::pi<float>() * 2.0f);
            decal_blood.get_transform().rotation = glm::quat(rot);
            decal_blood.get_transform().scale = glm::vec3(2.0f);

            auto &decal = decal_blood.add_component<DecalComponent>();
            decal.albedo_name = "decal_white";
            decal.normal_name = "decal_normal";
            decal.color_tint = Color(0.42f, 0.0f, 0.0f, 1.0f);
        }
        //do some randomness shit here
        //blood splatter component
        //blood_mesh_TRIANGLE_CLOUD
        //blood_pos
        //blood_norm
        //-26.510, -54.750, -38.470 offset -> ditch?
        //.03 scale

        //raycast down from hit, spawn a decal
    }

    void Scene::add_system(std::unique_ptr<System> system)
    {
        system->on_create();
        systems.emplace_back(std::move(system));
    }

    void Scene::initialize_systems()
    {
        add_system(std::make_unique<PhysicsSystem>());
        add_system(std::make_unique<PlayerControllerSystem>());
        add_system(std::make_unique<BulletSystem>());
        add_system(std::make_unique<NPCSystem>());
        add_system(std::make_unique<InteractionSystem>());
        add_system(std::make_unique<EditorCameraControllerSystem>());
        add_system(std::make_unique<TransformSystem>());
        add_system(std::make_unique<AnimationSystem>());
        add_system(std::make_unique<RagdollSystem>());
        add_system(std::make_unique<BloodSystem>());
        add_system(std::make_unique<RendererSystem>());
    }
}
