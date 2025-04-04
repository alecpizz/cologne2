//
// Created by alecpizz on 3/3/2025.
//

#include "Scene.h"
#include "Physics.h"
#include "renderer/Model.h"

namespace goon
{
    struct Scene::Impl
    {
        std::vector<std::unique_ptr<Model>> models = std::vector<std::unique_ptr<Model>>();
    };

    Scene::Scene()
    {
        _impl = new Impl();
        // auto &model = add_model(RESOURCES_PATH "backpack/backpack.glb", true);
        // model.get_transform()->set_translation(glm::vec3(0.0f, 2.0f, 10.0f));
        // auto &model2 = add_model(RESOURCES_PATH "Lantern.glb", false);
        // model2.get_transform()->set_translation(glm::vec3(0.0f, 10.0f, -10.0f));
        auto &model3 = add_model(RESOURCES_PATH "sponza/sponza2.glb", false);
        model3.get_transform()->set_scale(glm::vec3(.01f));
        // model3.get_transform()->set_rotation(glm::rotate(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
        // goon::physics::update_mesh_collider(&model);
        // goon::physics::update_mesh_collider(&model2);
        goon::physics::update_mesh_collider(&model3);
    }

    void Scene::update(float delta_time)
    {
    }

    Scene::~Scene()
    {
        delete _impl;
    }

    Model *Scene::get_model_by_index(size_t idx) const
    {
        return _impl->models[idx].get();
    }

    uint64_t Scene::get_model_count() const
    {
        return _impl->models.size();
    }

    Model &Scene::add_model(const char* path, bool flip_textures) const
    {
        return *_impl->models.emplace_back(std::make_unique<Model>(path, flip_textures));
    }
}
