//
// Created by alecpizz on 3/3/2025.
//

#include "Scene.h"
#include "Physics.h"
#include "renderer/Model.h"
#include "Animation.h"
#include "Animator.h"
#include "Engine.h"
#include "renderer/SkinnedModel.h"

namespace cologne
{
    glm::vec3 gun_offset_temp = glm::vec3(0.165f, -0.145f, -0.065f);
    glm::vec3 gun_offset_euler_temp = glm::vec3(0.0f, 90.0f, 0.0f);
    glm::vec3 gun_offset_scale_temp = glm::vec3(1.0f);
    Scene::Scene()
    {
        // auto &model = add_model(RESOURCES_PATH "backpack/backpack.glb", true);
        // model.get_transform()->set_translation(glm::vec3(0.0f, 2.0f, 10.0f));
        // auto &model2 = add_model(RESOURCES_PATH "Lantern.glb", false);
        // model2.get_transform()->set_translation(glm::vec3(0.0f, 10.0f, -10.0f));
        auto &model3 = add_model(RESOURCES_PATH "sponza/sponza2.glb", false);
        model3.get_transform()->set_scale(glm::vec3(.01f));
        auto bounds = model3.get_aabb();
        bounds.min *= model3.get_transform()->scale;
        bounds.max *= model3.get_transform()->scale;
        model3.set_aabb(bounds);
        auto &model = add_model(RESOURCES_PATH "glowCube.glb", false);
        model.set_gi_only(true);
        auto& skinned_model = add_skinned_model(RESOURCES_PATH "python/deagle.glb", "deagle");
        skinned_model.set_cast_shadows(false);
        _animations = Animation::get_animations(RESOURCES_PATH "python/deagle.glb", skinned_model);
        _animators.insert(std::make_pair("deagle", Animator(_animations[1])));

        auto& skinned_model2 = add_skinned_model(RESOURCES_PATH "man.glb", "man");
        skinned_model2.get_transform().set_scale(glm::vec3(0.9f));
        _animations.emplace_back(RESOURCES_PATH "man.glb", skinned_model2);
        _animators.insert(std::make_pair("man", Animator(_animations.back())));
        // model3.get_transform()->set_rotation(glm::rotate(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
        // cologne::physics::update_mesh_collider(&model);
        // cologne::physics::update_mesh_collider(&model2);
        cologne::physics::update_mesh_collider(&model3);
        re_calculate_bounds();
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
        _models.clear();
    }


    void Scene::update(float delta_time)
    {
        auto &model = _models[1];
        static float time = 0.0f;
        time += delta_time * 0.85;
        // -11 1 4 init pos
        // 13 1 4 final pos
        glm::vec3 new_pos = glm::lerp(glm::vec3(-11.0f, 1.0f, 4.0f),
            glm::vec3(13.0f, 1.0f, 4.0f), glm::abs(glm::cos(time)));
        model->get_transform()->set_translation(glm::vec3(new_pos));
        for (auto& anim : _animators)
        {
            anim.second.update_animation(delta_time);
        }

        glm::mat4 gun_mat = glm::mat4(1.0f);
        gun_mat = glm::translate(gun_mat, gun_offset_temp);
        gun_mat *= glm::toMat4(glm::quat(glm::radians(glm::vec3(gun_offset_euler_temp))));
        gun_mat *= glm::scale(gun_mat, gun_offset_scale_temp);
        gun_mat = glm::inverse(Engine::get_camera()->get_view_matrix()) * gun_mat;
        _skinned_models[0].get_transform().set_model_matrix(gun_mat);
    }

    AABB Scene::re_calculate_bounds()
    {
        _scene_bounds = {};
        for (auto &model: _models)
        {
            _scene_bounds.expand(model->get_aabb().min);
            _scene_bounds.expand(model->get_aabb().max);
        }
        return _scene_bounds;
    }

    AABB Scene::get_bounds() const
    {
        return _scene_bounds;
    }

    std::vector<Particles> & Scene::get_particles()
    {
        return _particles;
    }


    Model *Scene::get_model_by_index(size_t idx) const
    {
        return _models[idx].get();
    }

    uint64_t Scene::get_model_count() const
    {
        return _models.size();
    }

    Model &Scene::add_model(const char *path, bool flip_textures)
    {
        return *_models.emplace_back(std::make_unique<Model>(path, flip_textures));
    }

    SkinnedModel & Scene::add_skinned_model(const char *path, const char* name)
    {
        return _skinned_models.emplace_back(path, name);
    }


    std::vector<SkinnedModel> & Scene::get_skinned_models()
    {
        return _skinned_models;
    }


    std::unordered_map<std::string, Animator>& Scene::get_animators()
    {
        return _animators;
    }
}
