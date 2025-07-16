#include "DebugRenderer.h"

#include <engine/core/Engine.h>

#include "OpenGLDebugScope.h"
#include "engine/core/Input.h"
#include "../renderer/types/Shader.h"

namespace cologne
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

        void update_vertex_data(std::vector<DebugVertex> &vertices)
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
                allocated_buffer_size = static_cast<uint32_t>(bufferSize);
            }

            glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, vertices.data());

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void *) offsetof(DebugVertex, point));

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void *) offsetof(DebugVertex, color));
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            vertex_count = static_cast<uint32_t>(vertices.size());
        }

        void draw()
        {
            if (!is_drawing)
            {
                cmds.clear();
                return;
            }
            OpenGLDebugScope scope("DebugRenderer::draw");
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glPointSize(8.0f);

            shader->bind();

            update_vertex_data(cmds);

            cmds.clear();

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
        constexpr int sectors = 18;
        constexpr int stacks = 8;

        std::vector<std::vector<glm::vec3> > vertices(stacks + 1, std::vector<glm::vec3>(sectors + 1));
        for (int i = 0; i <= stacks; ++i)
        {
            float v = static_cast<float>(i) / static_cast<float>(stacks);
            float phi = glm::pi<float>() * v;

            for (int j = 0; j <= sectors; ++j)
            {
                float u = static_cast<float>(j) / static_cast<float>(sectors);
                float theta = 2.0f * glm::pi<float>() * u;

                float x = glm::cos(theta) * glm::sin(phi);
                float y = glm::cos(phi);
                float z = glm::sin(theta) * glm::sin(phi);

                vertices[i][j] = center + glm::vec3(x, y, z) * radius;
            }
        }

        for (int i = 0; i <= stacks; ++i)
        {
            for (int j = 0; j < sectors; ++j)
            {
                draw_line(vertices[i][j], vertices[i][j + 1], color);
            }
        }

        for (int j = 0; j <= sectors; ++j)
        {
            for (int i = 0; i < stacks; ++i)
            {
                draw_line(vertices[i][j], vertices[i + 1][j], color);
            }
        }
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

    void DebugRenderer::draw_aabb(glm::mat4 transform, glm::vec3 aabb_min, glm::vec3 aabb_max, glm::vec3 color)
    {
        glm::vec3 center = (aabb_max + aabb_min) * 0.5f;
        glm::vec3 extents = (aabb_max - aabb_min) * 0.5f;

        glm::vec3 world_center = glm::vec3(transform * glm::vec4(center, 1.0));
        glm::vec3 world_extents = glm::abs(extents.x * transform[0]) +
                             glm::abs(extents.y * transform[1]) +
                             glm::abs(extents.z * transform[2]);

        glm::vec3 min = world_center - world_extents;
        glm::vec3 max = world_center + world_extents;


        glm::vec3 FrontTopLeft = glm::vec4(min.x, max.y, max.z, 1.0f);
        glm::vec3 FrontTopRight = glm::vec4(max.x, max.y, max.z, 1.0f);
        glm::vec3 FrontBottomLeft = glm::vec4(min.x, min.y, max.z, 1.0f);
        glm::vec3 FrontBottomRight = glm::vec4(max.x, min.y, max.z, 1.0f);
        glm::vec3 BackTopLeft = glm::vec4(min.x, max.y, min.z, 1.0f);
        glm::vec3 BackTopRight =  glm::vec4(max.x, max.y, min.z, 1.0f);
        glm::vec3 BackBottomLeft =  glm::vec4(min.x, min.y, min.z, 1.0f);
        glm::vec3 BackBottomRight = glm::vec4(max.x, min.y, min.z, 1.0f);
        draw_line(FrontTopLeft, FrontTopRight, color);
        draw_line(FrontBottomLeft, FrontBottomRight, color);
        draw_line(BackTopLeft, BackTopRight, color);
        draw_line(BackBottomLeft, BackBottomRight, color);
        draw_line(FrontTopLeft, FrontBottomLeft, color);
        draw_line(FrontTopRight, FrontBottomRight, color);
        draw_line(BackTopLeft, BackBottomLeft, color);
        draw_line(BackTopRight, BackBottomRight, color);
        draw_line(FrontTopLeft, BackTopLeft, color);
        draw_line(FrontTopRight, BackTopRight, color);
        draw_line(FrontBottomLeft, BackBottomLeft, color);
        draw_line(FrontBottomRight, BackBottomRight, color);
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
