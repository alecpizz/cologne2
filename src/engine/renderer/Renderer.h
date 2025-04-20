#pragma once
#include "Light.h"
#include "engine/Scene.h"


namespace cologne
{
    class Scene;
    class Shader;

    class Renderer
    {
        friend class Engine;

    public:
        ~Renderer();

        Renderer(Renderer &&) = delete;

        Renderer(const Renderer &) = delete;

        Renderer &operator=(Renderer &&) = delete;

        Renderer &operator=(const Renderer &) = delete;

        void draw_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color);

        void draw_box(glm::vec3 center, glm::vec3 size, glm::vec3 color);

        void draw_sphere(glm::vec3 center, float radius, glm::vec3 color);

        void draw_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color);

        void draw_aabb(glm::mat4 transform, glm::vec3 min, glm::vec3 max, glm::vec3 color);

        void render_scene(Scene &scene);

        void window_resized(uint32_t width, uint32_t height);

        void reload_shaders();

        Shader* get_shader_by_name(const char* name);
        Light &get_directional_light() const;

        void set_directional_light(glm::vec3 position, glm::vec3 direction);

    private:
        Renderer();

        struct Impl;
        Impl *_impl;
    };
}
