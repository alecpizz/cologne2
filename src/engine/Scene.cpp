//
// Created by alecpizz on 3/3/2025.
//

#include "Scene.h"
#include "gpch.h"
#include "renderer/Model.h"

namespace goon
{
    struct Scene::Impl
    {
        std::vector<Model> models = std::vector<Model>();
    };

    Scene::Scene()
    {
        _impl = new Impl();
        auto &model = add_model(RESOURCES_PATH "backpack/backpack.glb");
        model.set_textures(RESOURCES_PATH "backpack",
                           "diffuse.jpg", "normal.png",
                           "ao.jpg", "roughness.jpg",
                           "specular.jpg");
        model.get_transform()->set_translation(glm::vec3(0.0f, 0.0f, 10.0f));
        model.get_transform()->set_rotation(glm::rotate(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    void Scene::update(float delta_time)
    {
    }

    Scene::~Scene()
    {
        delete _impl;
    }

    Model *Scene::get_models() const
    {
        return _impl->models.data();
    }

    uint64_t Scene::get_model_count() const
    {
        return _impl->models.size();
    }

    Model &Scene::add_model(const char *path) const
    {
        return _impl->models.emplace_back(path);
    }
}
