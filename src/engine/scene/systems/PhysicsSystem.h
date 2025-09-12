//
// Created by alecpizz on 8/13/25.
//
#pragma once
#include "System.h"

namespace cologne
{
    class PhysicsSystem : public System
    {
    public:
        void on_scene_start(Scene *scene) override;
        void on_update(Scene* scene, float dt) override;
        UpdateFlags get_update_flags() override
        {
            return RUNTIME;
        }
    };
}
