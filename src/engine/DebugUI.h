#pragma once
#include "engine/Engine.h"
#include "engine/Renderer.h"
#include "engine/Window.h"

namespace goon
{
    class DebugUI
    {
        friend class Engine;
    public:

        ~DebugUI();

        void clear();

        void present();

        DebugUI(DebugUI &&) = delete;

        DebugUI(const DebugUI &) = delete;

        DebugUI &operator=(DebugUI &&) = delete;

        DebugUI &operator=(const DebugUI &) = delete;

    private:
        DebugUI();
        struct Impl;
        Impl *_impl;
    };
}
