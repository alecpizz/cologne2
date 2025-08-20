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
        UpdateFlags get_update_flags() override
        {
            return static_cast<UpdateFlags>(EDITOR | RUNTIME);
        }
    private:
        void update_children(entt::entity parent, entt::registry& registry);
    };
}
