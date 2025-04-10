//
// Created by alecpizz on 3/29/2025.
//

#include "TextRenderer.h"
#include <stb_truetype/stb_truetype.h>
#include <fstream>

namespace cologne
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 uv;
    };

    struct TextRenderer::Impl
    {
        uint32_t texture_handle;
        uint32_t vao;
        uint32_t vbo;

        void load_font(const std::string &font_path)
        {

        }

        void draw_text(const std::string &text, const glm::vec3 &position, const glm::vec4 &color, float size)
        {
            for (auto &c : text)
            {

            }
        }
    };

    TextRenderer::~TextRenderer()
    {
        glDeleteTextures(0, &_impl->texture_handle);
        delete _impl;
        LOG_INFO("Text renderer destroyed");
    }

    void TextRenderer::draw_text(const char *text, glm::vec3 position, glm::vec4 color, float size)
    {
        _impl->draw_text(text, position, color, size);
    }

    void TextRenderer::present()
    {
    }

    TextRenderer::TextRenderer(const char *font_path)
    {
        _impl = new Impl();
        _impl->load_font(font_path);
    }
}
