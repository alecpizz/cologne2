//
// Created by alecpizz on 3/29/2025.
//

#include "TextRenderer.h"
#include <stb_truetype/stb_truetype.h>
#include <fstream>

namespace goon
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
        std::vector<Vertex> vertices;
#define FIRST_CHAR 32
#define CHAR_AMOUNT 95
        const uint32_t first_char = FIRST_CHAR;
        const uint32_t last_char = CHAR_AMOUNT;
        float font_size = 64.0f;
        stbtt_packedchar packedChars[FIRST_CHAR];
        stbtt_aligned_quad aligned_quads[CHAR_AMOUNT];

        void load_font(const std::string &font_path)
        {
            std::ifstream input_stream(font_path, std::ios::binary);
            input_stream.seekg(0, std::ios::end);
            auto &&size = input_stream.tellg();
            input_stream.seekg(0, std::ios::beg);

            uint8_t *font_data = new uint8_t[static_cast<size_t>(size)];
            input_stream.read(reinterpret_cast<char *>(font_data), size);
            int32_t font_count = stbtt_GetNumberOfFonts(font_data);
            if (font_count == -1)
            {
                LOG_ERROR("Font file doesn't have valid font data!");
            } else
            {
                LOG_INFO("Font file contains %d font(s)", font_count);
            }

            uint32_t width = 1024, height = 1024;
            uint8_t *font_atlas_bitmap = new uint8_t[width * height];

            stbtt_pack_context ctx;
            stbtt_PackBegin(&ctx, font_atlas_bitmap, width, height, 0, 1, nullptr);
            stbtt_PackFontRange(&ctx, font_data, 0, font_size, first_char,
                                last_char, packedChars);
            stbtt_PackEnd(&ctx);
            for (size_t i = 0; i < last_char; i++)
            {
                float x, y;
                stbtt_GetPackedQuad(packedChars, width, height, i,
                                    &x, &y, &aligned_quads[i], 0);
            }

            glGenTextures(1, &texture_handle);
            glBindTexture(GL_TEXTURE_2D, texture_handle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, font_atlas_bitmap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glBindTexture(GL_TEXTURE_2D, 0);

            delete[] font_atlas_bitmap;
            delete[] font_data;

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  reinterpret_cast<void *>(offsetof(Vertex, position)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  reinterpret_cast<void *>(offsetof(Vertex, color)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  reinterpret_cast<void *>(offsetof(Vertex, color)));

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, 60000 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
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
        _impl->vertices.clear();
    }

    TextRenderer::TextRenderer(const char *font_path)
    {
        _impl = new Impl();
        _impl->load_font(font_path);
    }
}
