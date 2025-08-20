//
// Created by alecpizz on 8/19/25.
//
#pragma once
#include "System.h"

namespace cologne
{
    class InteractionSystem : public System
    {
        void on_update(float dt) override;
    };
}
