//
// Created by alecpizz on 8/13/25.
//
#pragma once
#include <entt/entt.hpp>

namespace cologne
{
    class Scene;

    class System
    {
    public:
        virtual ~System() = default;

        virtual void on_create(Scene *scene)
        {
            _scene = scene;
        }

        virtual void on_update(float dt) = 0;

        virtual void on_destroy()
        {
        }

    protected:
        Scene *_scene = nullptr;
    };
}
