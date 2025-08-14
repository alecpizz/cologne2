//
// Created by alecpizz on 8/14/25.
//
#pragma once
#include "System.h"

namespace cologne
{
    class EditorCameraControllerSystem : public System
    {
    public:
        void on_update(float dt) override;
    private:
        void free_cam(entt::registry& registry, entt::entity camera, float dt);
        void pan(entt::registry& registry, entt::entity camera, float dt);
    };
}
