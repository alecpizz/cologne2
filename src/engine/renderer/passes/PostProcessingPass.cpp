//
// Created by alecpizz on 8/11/25.
//
#include <engine/core/Engine.h>
#include <engine/core/Time.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>
#include <engine/editor/Editor.h>
#include <engine/renderer/OpenGLDebugScope.h>

namespace cologne
{
    void Renderer::post_processing_pass()
    {
        OpenGLDebugScope scope("Renderer::post_processing_pass");
        auto output_fbo = get_framebuffer_by_name("output");
        auto shader = get_shader_by_name("post_processing");
        if (!output_fbo || !shader)
        {
            return;
        }
        output_fbo->bind();
        output_fbo->set_viewport();
        static float time = 0.0f;
        time += Time::DeltaTime;
        shader->set_float("time", time);
        glBindImageTexture(0, output_fbo->get_color_attachment_handle_by_name("color"),
            0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindTextureUnit(1, _bloom_texture);
        shader->bind();
        uint32_t work_group_size = 32;
        uint32_t width = Engine::get_render_target_width();
        uint32_t height = Engine::get_render_target_height();
        uint32_t num_x = (width + work_group_size - 1) / work_group_size;
        uint32_t num_y = (height + work_group_size - 1) / work_group_size;
        shader->dispatch(num_x, num_y, 1);
        shader->wait(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
}
