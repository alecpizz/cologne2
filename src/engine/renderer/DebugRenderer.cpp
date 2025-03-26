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
        std::vector<debug_cmd> cmds;
        std::vector<float> vertices;
        std::vector<uint32_t> indices;
        uint32_t VAO, VBO, EBO;

        void init()
        {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

        }

        void draw()
        {
            vertices.clear();
            indices.clear();
            uint32_t index = 0;
            for (auto& cmd : cmds)
            {
                vertices.push_back(cmd.p1.x);
                vertices.push_back(cmd.p1.y);
                vertices.push_back(cmd.p1.z);
                vertices.push_back(cmd.color.r);
                vertices.push_back(cmd.color.g);
                vertices.push_back(cmd.color.b);

                vertices.push_back(cmd.p2.x);
                vertices.push_back(cmd.p2.y);
                vertices.push_back(cmd.p2.z);
                vertices.push_back(cmd.color.r);
                vertices.push_back(cmd.color.g);
                vertices.push_back(cmd.color.b);

                indices.push_back(index);
                indices.push_back(index + 1);
                index += 2;
            }

        }
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
        _impl->cmds.clear();
    }

    void DebugRenderer::present()
    {
        _impl->draw();
    }

    DebugRenderer::DebugRenderer()
    {
        LOG_INFO("Starting DebugRenderer");
        _impl = new Impl();
        _impl->init();
    }

    DebugRenderer::~DebugRenderer()
    {
        delete _impl;
    }
}
