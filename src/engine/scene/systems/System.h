//
// Created by alecpizz on 8/13/25.
//
#pragma once
#include <entt/entt.hpp>

namespace cologne
{
    class Scene;

    enum UpdateFlags : int
    {
        NONE = 0,
        EDITOR = 1 << 0,
        RUNTIME = 1 << 1
    };
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

        virtual UpdateFlags get_update_flags() = 0;

    protected:
        Scene *_scene = nullptr;
    };
}
