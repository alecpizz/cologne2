//
// Created by alecpizz on 9/11/25.
//
#pragma once
#include "System.h"

namespace cologne
{
    class RagdollSystem : public System
    {
    public:
        void on_update(Scene *scene, float dt) override;
        UpdateFlags get_update_flags() override
        {
            return RUNTIME;
        }
    };
}
