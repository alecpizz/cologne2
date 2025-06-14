//
// Created by alecpizz on 6/9/25.
//
#include <engine/core/Engine.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>

namespace cologne
{
    struct BloomMip
    {
        glm::vec2 size = glm::vec2(0.0f);
        glm::ivec2 sizei = glm::ivec2(0.0);
        uint32_t texture = 0;
    };

    std::vector<BloomMip> mips;
    uint32_t bloom_fbo = 0;

    void Renderer::init_bloom(uint32_t width, uint32_t height)
    {
        if (bloom_fbo != 0)
        {
            glDeleteFramebuffers(1, &bloom_fbo);
        }
        constexpr uint32_t mip_chain_length = 5;
        mips.clear();
        mips.reserve(5);
        glm::vec2 mip_size;
        if (width == 0)
        {
            mip_size = Engine::get_window()->get_dimensions();
        }
        else
        {
            mip_size = glm::vec2(width, height);
        }
        glm::ivec2 mip_int_size = glm::ivec2(mip_size);

        glGenFramebuffers(1, &bloom_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);

        for (auto& bloom_mip : mips)
        {
            if (bloom_mip.texture == 0)
            {
                continue;
            }
            glDeleteTextures(1, &bloom_mip.texture);
        }
        mips.clear();
        for (uint32_t i = 0; i < mip_chain_length; i++)
        {
            BloomMip mip;
            mip_size *= 0.5f;
            mip_int_size /= 2;

            mip.size = mip_size;
            mip.sizei = mip_int_size;

            glGenTextures(1, &mip.texture);
            glBindTexture(GL_TEXTURE_2D, mip.texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
                         mip.sizei.x, mip.sizei.y, 0, GL_RGB, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if (i == 0)
            {
                _bloom_texture = mip.texture;
            }
            mips.emplace_back(mip);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, mips[0].texture, 0);
        uint32_t attachments[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        Engine::get_debug_ui()->add_image_entry("Bloom result", mips[0].texture, mips[0].size);
    }

    void Renderer::bloom_pass()
    {
        auto gbuffer = get_framebuffer_by_name("gbuffer");
        auto upsample_shader = get_shader_by_name("upsample");
        auto downsample_shader = get_shader_by_name("downsample");
        if (!gbuffer || !upsample_shader || !downsample_shader)
        {
            return;
        }
        downsample_shader->bind();
        downsample_shader->set_vec2("src_resolution", 1.0f / Engine::get_window()->get_dimensions());
        glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
        glBindTextureUnit(0, gbuffer->get_color_attachment_handle_by_name("emission"));
        //downsamples
        for (auto &mip: mips)
        {
            glViewport(0, 0, mip.sizei.x, mip.sizei.y);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, mip.texture, 0);

            render_quad();
            downsample_shader->set_vec2("src_resolution", 1.0f / mip.size);
            glBindTextureUnit(0, mip.texture);
        }

        //upsamples
        constexpr float filter_radius = 0.005f;
        upsample_shader->bind();
        upsample_shader->set_float("filter_radius", filter_radius);
        upsample_shader->set_float("src_aspect_ratio",
                                   static_cast<float>(Engine::get_window()->get_width()) / static_cast<float>(
                                       Engine::get_window()->get_height()));

        //todo: check blending here
        for (int i = mips.size() - 1; i > 0; i--)
        {
            auto &mip = mips[i];
            auto &next_mip = mips[i - 1];

            glBindTextureUnit(0, mip.texture);
            glViewport(0, 0, next_mip.sizei.x, next_mip.sizei.y);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, next_mip.texture, 0);
            render_quad();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
    }
}
