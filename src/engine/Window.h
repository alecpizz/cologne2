﻿#pragma once

namespace cologne
{
    class Window
    {
        friend class Engine;
    public:
        ~Window();

        Window(Window &&) = delete;

        Window(const Window &) = delete;

        Window &operator=(Window &&) = delete;

        Window &operator=(const Window &) = delete;

        uint32_t get_width() const;

        uint32_t get_height() const;

        void clear() const;

        void resize() const;

        void maximize() const;

        void present() const;

        void minimize();

        void hide_mouse() const;
        void show_mouse() const;

    private:
        Window(uint32_t width, uint32_t height);

        struct Impl;
        Impl *_impl;
    };
}
