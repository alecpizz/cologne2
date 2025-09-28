#pragma once

namespace cologne
{
    struct DebugVertex
    {
        glm::vec3 point;
        glm::vec3 color;
    };
    class DebugRenderer
    {
    public:
        DebugRenderer();

        ~DebugRenderer();

        DebugRenderer(DebugRenderer &&) = delete;

        DebugRenderer(const DebugRenderer &) = delete;

        DebugRenderer &operator=(DebugRenderer &&) = delete;

        DebugRenderer &operator=(const DebugRenderer &) = delete;

        void draw_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color);

        void draw_box(glm::mat4 transform, glm::vec3 min, glm::vec3 max, glm::vec3 color);

        void draw_sphere(glm::vec3 center, float radius, glm::vec3 color);

        void draw_point(glm::vec3 p, glm::vec3 color);

        void draw_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color);

        void draw_aabb(glm::mat4 transform, glm::vec3 aabb_min, glm::vec3 aabb_max, glm::vec3 color);

        void present();

    private:
        struct Impl;
        Impl *_impl;
    };
}
