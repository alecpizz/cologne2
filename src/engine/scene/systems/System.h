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

        virtual void on_create()
        {
        }

        virtual void on_scene_start(Scene *scene)
        {
        }

        virtual void on_update(Scene *scene, float dt) = 0;

        virtual void on_destroy()
        {
        }

        virtual void on_scene_exit(Scene *scene)
        {
        }

        virtual UpdateFlags get_update_flags() = 0;
    };
}
