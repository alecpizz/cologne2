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
        void on_update(float dt) override;
    };
}
