//
// Created by alecpizz on 8/19/25.
//
#pragma once
#include "System.h"

namespace cologne
{
    class InteractionSystem : public System
    {
    public:
        UpdateFlags get_update_flags() override
        {
            return RUNTIME;
        }
        void on_update(float dt) override;
    };
}
