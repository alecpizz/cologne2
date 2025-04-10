#include "DebugRenderer.h"

#include <engine/Engine.h>
#include "engine/Input.h"
#include "Shader.h"

namespace goon
{
    struct DebugVertex
    {
        glm::vec3 point;
        glm::vec3 color;
    };
    struct DebugCmd
    {
        glm::vec3 p1, p2;
        glm::vec3 color;
    };


    struct DebugRenderer::Impl
    {
        std::vector<DebugVertex> cmds;
        uint32_t VAO, VBO;
        uint32_t allocated_buffer_size;
        uint32_t vertex_count;
        std::unique_ptr<Shader> shader = nullptr;
        bool is_drawing = true;

        void init()
        {
            is_drawing = true;
            shader = std::make_unique<Shader>(RESOURCES_PATH "shaders/debug.vert", RESOURCES_PATH "shaders/debug.frag");
        }

        void update_vertex_data(std::vector<DebugVertex>& vertices)
        {
            if (VAO == 0)
            {
                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);
            }
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            size_t bufferSize = vertices.size() * sizeof(DebugVertex);
            if (bufferSize > allocated_buffer_size)
            {
                glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
                allocated_buffer_size = bufferSize;
            }

            glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, vertices.data());

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, point));

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            vertex_count = vertices.size();
        }

        void draw()
        {
            if (!is_drawing)
            {
                cmds.clear();
                return;
            }
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glPointSize(8.0f);

            shader->bind();

            update_vertex_data(cmds);

            cmds.clear();

            shader->set_mat4("view", glm::value_ptr(Engine::get_camera()->get_view_matrix()));
            shader->set_mat4("projection", glm::value_ptr(Engine::get_camera()->get_projection_matrix()));
            glBindVertexArray(VAO);
            glDrawArrays(GL_LINES, 0, vertex_count);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glEnable(GL_BLEND);
        }
    };


    void DebugRenderer::draw_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color)
    {
        _impl->cmds.emplace_back(DebugVertex(p1, color));
        _impl->cmds.emplace_back(DebugVertex(p2, color));
    }

    void DebugRenderer::draw_box(glm::vec3 center, glm::vec3 size, glm::vec3 color)
    {
    }

    void DebugRenderer::draw_sphere(glm::vec3 center, float radius, glm::vec3 color)
    {
    }

    void DebugRenderer::draw_point(glm::vec3 p, glm::vec3 color)
    {
        _impl->cmds.emplace_back(DebugVertex(p, color));
        _impl->cmds.emplace_back(DebugVertex(p + glm::vec3(0.0f, 0.02f, 0.0f), color));
    }

    void DebugRenderer::draw_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color)
    {
        draw_line(p1, p2, color);
        draw_line(p2, p3, color);
        draw_line(p3, p1, color);
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
