//
// Created by alecpizz on 9/3/25.
//
#pragma once
#include <engine/scene/systems/System.h>

namespace cologne
    {
        class AnimationSystem2 : public System
{
          public:
                void on_update(Scene* scene, float dt) override;
                UpdateFlags get_update_flags() override
                {
                    return RUNTIME;
                }

          };
}