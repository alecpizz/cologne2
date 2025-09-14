//
// Created by alecpizz on 9/13/25.
//

#include "NavmeshDebugDrawer.h"

#include <engine/renderer/DebugRenderer.h>
#include <engine/renderer/Renderer.h>

namespace cologne
{
    static std::vector<DebugVertex> vertex_buffer;

    NavmeshDebugDrawer::NavmeshDebugDrawer() : _current_primitive(DU_DRAW_LINES)
    {
    }

    void NavmeshDebugDrawer::depthMask(bool state)
    {
    }

    void NavmeshDebugDrawer::texture(bool state)
    {
    }

    void NavmeshDebugDrawer::begin(duDebugDrawPrimitives prim, float size)
    {
        _current_primitive = prim;
        vertex_buffer.clear();
    }

    void NavmeshDebugDrawer::vertex(const float *pos, unsigned int color)
    {
        vertex(pos[0], pos[1], pos[2], color);
    }

    void NavmeshDebugDrawer::vertex(const float *pos, unsigned color, const float *uv)
    {
        vertex(pos[0], pos[1], pos[2], color);
    }

    void NavmeshDebugDrawer::vertex(const float x, const float y, const float z, unsigned color, const float u,
                                    const float v)
    {
        vertex(x, y, z, color);
    }

    void NavmeshDebugDrawer::vertex(const float x, const float y, const float z, unsigned int color)
    {
        vertex_buffer.emplace_back(DebugVertex({x, y, z}, du_to_glm_color(color)));
    }


    void NavmeshDebugDrawer::end()
    {
        if (vertex_buffer.empty())
        {
            return;
        }

        if (_current_primitive == DU_DRAW_TRIS)
        {
            for (size_t i = 0; i < vertex_buffer.size(); i += 3)
            {
                Renderer::draw_triangle(vertex_buffer[i].point, vertex_buffer[i + 1].point,
                                        vertex_buffer[i + 2].point, vertex_buffer[i].color);
            }
        }
        else if (_current_primitive == DU_DRAW_LINES)
        {
            for (size_t i = 0; i < vertex_buffer.size(); i += 2)
            {
                Renderer::draw_line(vertex_buffer[i].point,
                                    vertex_buffer[i + 1].point, vertex_buffer[i].color);
            }
        }
        else if (_current_primitive == DU_DRAW_POINTS)
        {
            for (const auto &vert: vertex_buffer)
            {
                Renderer::draw_point(vert.point, vert.color);
            }
        }

        vertex_buffer.clear();
    }

    glm::vec4 NavmeshDebugDrawer::du_to_glm_color(unsigned int color)
    {
        const float r = ((color >> 24) & 0xFF) / 255.0f;
        const float g = ((color >> 16) & 0xFF) / 255.0f;
        const float b = ((color >> 8) & 0xFF) / 255.0f;
        const float a = (color & 0xFF) / 255.0f;
        return {r, g, b, a};
    }
}
