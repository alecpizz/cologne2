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

    void Scene::add_model(const char *path) const
    {
        _impl->models.emplace_back(path);
    }
}
