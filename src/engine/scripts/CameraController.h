#pragma once
#include <engine/scene/ScriptableEntity.h>
#include <engine/scene/Components.h>

namespace cologne
{
    class CameraController : public ScriptableEntity
    {
    private:
        size_t idx = 0;
    protected:
        void on_create() override
        {
        }

        void on_destroy() override
        {

        }

        void on_update(float dt) override
        {
        }
    };
}
