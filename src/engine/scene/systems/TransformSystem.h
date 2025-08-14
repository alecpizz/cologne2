//
// Created by alecpizz on 8/13/25.
//
#pragma once
#include "System.h"

namespace cologne
{
    class TransformSystem : public System
    {
    public:
        void on_update(float dt) override;
    private:
        void update_children(entt::entity parent, entt::registry& registry);
    };
}
