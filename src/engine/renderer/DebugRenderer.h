#pragma once

namespace goon
{
    class DebugRenderer
    {
        friend class Engine;
    public:
        ~DebugRenderer();
        DebugRenderer(DebugRenderer &&) = delete;

        DebugRenderer(const DebugRenderer &) = delete;

        DebugRenderer &operator=(DebugRenderer &&) = delete;

        DebugRenderer &operator=(const DebugRenderer &) = delete;

        void draw_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color);
        void draw_box(glm::vec3 center, glm::vec3 size, glm::vec3 color);
        void draw_sphere(glm::vec3 center, float radius, glm::vec3 color);
        void draw_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color);
        void clear();
        void present();
    private:
        DebugRenderer();
        struct Impl;
        Impl *_impl;
    };
}
