#pragma once
#include "Light.h"
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
        Light& get_directional_light() const;
        void set_directional_light(glm::vec3 direction);

    private:
        Renderer();

        struct Impl;
        Impl *_impl;
    };
}
