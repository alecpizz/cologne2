#pragma once

namespace goon
{
    class DebugUI
    {
        friend class Engine;
    public:

        ~DebugUI();

        void clear();

        void present();

        void add_float_entry(const char* name, float& value);

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
