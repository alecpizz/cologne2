//
// Created by alecpizz on 3/29/2025.
//

#include "TextRenderer.h"
#include <ft2build.h>
#include <engine/Engine.h>

#include "Shader.h"

#include FT_FREETYPE_H

namespace cologne
{
    struct Character
    {
        uint32_t texture_id;
        glm::ivec2 size;
        glm::ivec2 bearing;
        uint32_t advance;
    };

    struct DrawCmd
    {
        std::string text;
        glm::vec3 position;
        glm::vec4 color;
        float scale;
    };

    std::unordered_map<char, Character> characters;
    std::unique_ptr<Shader> text_shader;
    std::vector<DrawCmd> draw_cmds;
    uint32_t vao, vbo;

    TextRenderer::~TextRenderer()
    {
    }

    void TextRenderer::draw_text(const char *text, glm::vec3 position, glm::vec4 color, float size)
    {
        draw_cmds.emplace_back(DrawCmd(std::string(text), position, color, size));
    }

    void TextRenderer::present()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        text_shader->bind();
        glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glm::mat4 projection = glm::ortho(0.0f,
                                          static_cast<float>(Engine::get_window()->get_width()),
                                          0.0f, static_cast<float>(Engine::get_window()->get_height()));
        text_shader->set_mat4("projection", glm::value_ptr(projection));
        glBindVertexArray(vao);
        for (auto &cmd: draw_cmds)
        {
            text_shader->set_vec4("textColor", glm::value_ptr(cmd.color));
            for (std::string::const_iterator c = cmd.text.begin(); c != cmd.text.end(); ++c)
            {
                Character ch = characters[*c];

                float xpos = cmd.position.x + ch.bearing.x * cmd.scale;
                float ypos = cmd.position.y - (ch.size.y - ch.bearing.y) * cmd.scale;

                float w = ch.size.x * cmd.scale;
                float h = ch.size.y * cmd.scale;
                // update VBO for each character
                float vertices[6][4] = {
                    {xpos, ypos + h, 0.0f, 0.0f},
                    {xpos, ypos, 0.0f, 1.0f},
                    {xpos + w, ypos, 1.0f, 1.0f},

                    {xpos, ypos + h, 0.0f, 0.0f},
                    {xpos + w, ypos, 1.0f, 1.0f},
                    {xpos + w, ypos + h, 1.0f, 0.0f}
                };

                glBindTextureUnit(0, ch.texture_id);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                cmd.position.x += (ch.advance >> 6) * cmd.scale;
            }
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_BLEND);
        draw_cmds.clear();
    }

    TextRenderer::TextRenderer(const char *font_path)
    {
        FT_Library ft;
        if (FT_Init_FreeType(&ft))
        {
            LOG_ERROR("Could not init FreeType library");
            return;
        }

        FT_Face face;
        if (FT_New_Face(ft, font_path, 0, &face))
        {
            LOG_ERROR("Failed to load font at path %s", font_path);
            return;
        }

        FT_Set_Pixel_Sizes(face, 0, 48);

        if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
        {
            LOG_ERROR("Failed to load glyph");
            return;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned char c = 0; c < 128; c++)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                LOG_ERROR("Failed to load glyph %c", c);
                continue;
            }
            if (face->glyph->bitmap.rows == 0 || face->glyph->bitmap.width == 0)
            {
                LOG_WARN("Character %c had a width/height of 0!", c);
                continue;
            }
            uint32_t texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
            };
            characters.insert(std::pair<char, Character>(c, character));
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        text_shader = std::make_unique<Shader>(RESOURCES_PATH "shaders/text.vert",
                                               RESOURCES_PATH "shaders/text.frag");

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}
