//
// Created by alecpizz on 3/3/2025.
//

#include "Scene.h"

#include <engine/animation/Animation.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/physics/Physics.h>
#include <engine/util/FileUtil.h>

#include "Components.h"
#include "Entity.h"

namespace cologne
{
    glm::vec3 gun_offset_temp = glm::vec3(0.165f, -0.145f, -0.065f);
    glm::vec3 gun_offset_euler_temp = glm::vec3(0.0f, 90.0f, 0.0f);
    glm::vec3 gun_offset_scale_temp = glm::vec3(1.0f);

    Scene::Scene()
    {
        //Create entities
        Entity sponza = create_entity("sponza");
        sponza.get_component<TransformComponent>().scale = glm::vec3(0.01f);
        sponza.add_component<ModelComponent>(AssetManager::get_model_index_by_name("sponza2"), false);

        auto model = AssetManager::get_model_by_name("sponza2");
        auto tr = sponza.get_component<TransformComponent>();
        for (auto &mesh: model->get_meshes())
        {
            Entity collider = create_entity(mesh.get_name());
            collider.get_component<TransformComponent>() = tr;
            uint32_t body_id = Physics::create_static_mesh_collider(tr, mesh.get_vertices(), mesh.get_indices());
            collider.add_component<StaticColliderComponent>(body_id);
        }

        Entity glowCube = create_entity("glowing cube");
        glowCube.get_component<TransformComponent>().position = glm::vec3(0.0f, 1.0f, 4.5f);
        glowCube.add_component<ModelComponent>(AssetManager::get_model_index_by_name("glowCube"), true);

        Entity man = create_entity("man");
        man.add_component<SkinnedModelComponent>(AssetManager::get_skinned_model_index_by_name("man"));
        man.add_component<AnimatorComponent>(AssetManager::get_animation_index_by_name("Idle"));
        re_calculate_bounds();
        // auto& skinned_model = add_skinned_model(RESOURCES_PATH "python/deagle.glb");
        // skinned_model.set_cast_shadows(false);
        // _animations = FileUtil::import_animations(RESOURCES_PATH "python/deagle.glb");
        // for (auto & animation : _animations)
        // {
        //     animation.read_missing_bones(skinned_model)
        // }
        // _animators.insert(std::make_pair(skinned_model.get_name(), Animator(_animations[1])));
        // auto& skinned_model2 = add_skinned_model(RESOURCES_PATH "man.glb");
        // skinned_model2.get_transform().set_scale(glm::vec3(0.9f));
        // //where the fuck should animations live lol
        // std::vector<Animation> animations2 = FileUtil::import_animations(RESOURCES_PATH "man.glb", skinned_model2);
        // _animations.insert(_animations.end(), animations2.begin(), animations2.end());
        // _animators.insert(std::make_pair(skinned_model2.get_name(), Animator(_animations.back())));
        //
        // for (auto & static_model : _models)
        // {
        //     Physics::create_static_mesh_collider(static_model);
        // }

        LOG_INFO("Scene bounds are min (%f, %f, %f), max (%f, %f, %f)", _scene_bounds.min.x, _scene_bounds.min.y,
                 _scene_bounds.min.z, _scene_bounds.max.z, _scene_bounds.max.y, _scene_bounds.max.z);
        LOG_INFO("Scene size is (%f, %f, %f)", _scene_bounds.size().x, _scene_bounds.size().y, _scene_bounds.size().z);
        _particles.emplace_back(Particles());
        _particles[0].init(_scene_bounds, 20);
        Engine::get_debug_ui()->add_vec3_entry("gun position", gun_offset_temp);
        Engine::get_debug_ui()->add_vec3_entry("gun euler", gun_offset_euler_temp);
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

        //game logic here
        auto animators = _registry.view<AnimatorComponent>();
        for (auto entity : animators)
        {
            auto& animator = _registry.get<AnimatorComponent>(entity);
            animator.update_animation(delta_time);
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
            Engine::get_renderer()->submit_render_item(RenderItem(model, tr, m.gi_only));
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
                auto& animator = _registry.get<AnimatorComponent>(entity);
                item.bones = animator.get_bones();
            }
            SkinnedModel *skinned_model = AssetManager::get_skinned_model_by_index(m.id);
            item.skinned_model = skinned_model;
            item.transform = tr;
            Engine::get_renderer()->submit_skinned_render_item(item);
        }
        //update logic

        //update animations


        // auto &model = _models[1];
        // static float time = 0.0f;
        // time += delta_time * 0.85;
        // // -11 1 4 init pos
        // // 13 1 4 final pos
        // glm::vec3 new_pos = glm::lerp(glm::vec3(-11.0f, 1.0f, 4.0f),
        // glm::vec3(13.0f, 1.0f, 4.0f), glm::abs(glm::cos(time)));
        // model.get_transform().set_translation(glm::vec3(new_pos));

        // for (auto& anim : _animators)
        // {
        //     anim.second.update_animation(delta_time);
        // }
        //
        // glm::mat4 gun_mat = glm::mat4(1.0f);
        // gun_mat = glm::translate(gun_mat, gun_offset_temp);
        // gun_mat *= glm::toMat4(glm::quat(glm::radians(glm::vec3(gun_offset_euler_temp))));
        // gun_mat *= glm::scale(gun_mat, gun_offset_scale_temp);
        // gun_mat = glm::inverse(Engine::get_camera()->get_view_matrix()) * gun_mat;
        // _skinned_models[0].get_transform().set_model_matrix(gun_mat);
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
}
