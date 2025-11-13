//
// Created by alecpizz on 11/12/25.
//
#pragma once
#include "System.h"

namespace cologne
{
    class AudioSystem : public System
    {
    public:
        void on_create() override;
        void on_scene_start(Scene *scene) override;
        void on_scene_exit(Scene *scene) override;
        void on_update(Scene *scene, float dt) override;
        UpdateFlags get_update_flags() override
        {
            return RUNTIME;
        }
    };
}
