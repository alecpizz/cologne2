#include "DebugRenderer.h"
#include "gpch.h"

namespace goon
{
    struct debug_cmd
    {
        glm::vec3 p1, p2;
        glm::vec3 color;
    };

    struct DebugRenderer::Impl
    {
        uint32_t vao;
        std::vector<debug_cmd> cmds;
    };


    void DebugRenderer::draw_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color)
    {
        _impl->cmds.emplace_back(debug_cmd{p1, p2, color});
    }

    void DebugRenderer::draw_box(glm::vec3 center, glm::vec3 size, glm::vec3 color)
    {
    }

    void DebugRenderer::draw_sphere(glm::vec3 center, float radius, glm::vec3 color)
    {
    }

    void DebugRenderer::draw_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color)
    {
        _impl->cmds.emplace_back(debug_cmd{p1, p2, color});
        _impl->cmds.emplace_back(debug_cmd{p2, p3, color});
        _impl->cmds.emplace_back(debug_cmd{p3, p1, color});
    }

    void DebugRenderer::clear()
    {
    }

    void DebugRenderer::present()
    {
    }

    DebugRenderer::DebugRenderer()
    {
        LOG_INFO("Starting DebugRenderer");
        _impl = new Impl();
    }

    DebugRenderer::~DebugRenderer()
    {
        delete _impl;
    }
}
