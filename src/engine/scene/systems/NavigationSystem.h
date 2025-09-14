//
// Created by alecpizz on 9/14/25.
//
#pragma once
#include <engine/scene/systems/System.h>
namespace cologne
{
    class NavigationSystem : public System
    {
    public:
        void on_scene_start(Scene *scene) override;
        void on_update(Scene *scene, float dt) override;

        UpdateFlags get_update_flags() override
        {
            return static_cast<UpdateFlags>( RUNTIME);
        }
    };
}
