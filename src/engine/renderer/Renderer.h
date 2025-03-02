#pragma once

namespace goon
{
    class Renderer
    {
        friend class Engine;
    public:
        ~Renderer();
        Renderer(Renderer &&) = delete;

        Renderer(const Renderer &) = delete;

        Renderer &operator=(Renderer &&) = delete;

        Renderer &operator=(const Renderer &) = delete;
        void render();
    private:
        Renderer();
        struct Impl;
        Impl *_impl;
    };
}