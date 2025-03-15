#pragma once
#include "engine/Scene.h"

class Scene;


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
        void render_scene(Scene &scene);
        void reload_shaders();

    private:
        Renderer();

        struct Impl;
        Impl *_impl;
    };
}
