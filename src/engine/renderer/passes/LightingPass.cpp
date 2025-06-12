//
// Created by alecpizz on 5/3/2025.
//
#include <engine/core/Engine.h>
#include <engine/core/Time.h>
#include <engine/renderer/OpenGLDebugScope.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>
#include <engine/renderer/types/SSBO.h>


namespace cologne
{
    void Renderer::lit_pass()
    {
        OpenGLDebugScope scope("Renderer::lit_pass");
        auto shader = get_shader_by_name("lit");
        auto output_fbo = get_framebuffer_by_name("output");
        auto gbuffer_fbo = get_framebuffer_by_name("gbuffer");
        output_fbo->bind();
        output_fbo->set_viewport();
        // glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader->bind();

        // env_irradiance.bind(IRRADIANCE_INDEX);
        // env_prefilter.bind(PREFILTER_INDEX);
        // env_brdf.bind(BRDF_INDEX);
        update_shadow(*shader);

        shader->set_int("voxel_grid_size", _voxel_data.voxel_dimensions);
        shader->set_vec3("voxel_offset", (_voxel_data.voxel_offset));
        auto bounds = Engine::get_scene()->get_bounds();
        const glm::vec3 size = bounds.size(); //THIS IS WRONG!
        const float world_size = glm::max(size.x, glm::max(size.y, size.z));
        const float texelSize = 1.0f / _voxel_data.voxel_dimensions;
        const float voxel_size = world_size * texelSize;
        shader->set_float("voxel_size", voxel_size);
        auto min = bounds.min;
        auto max = bounds.max;
        shader->set_vec3("grid_min", (min));
        shader->set_vec3("grid_max", (max));
        static float time = 0.0f;
        time += Time::DeltaTime;
        shader->set_float("time", time);
        shader->set_bool("indirect_lighting_active", _apply_indirect_lighting);
        glBindTextureUnit(0, gbuffer_fbo->get_color_attachment_handle_by_name("position"));
        glBindTextureUnit(1, gbuffer_fbo->get_color_attachment_handle_by_name("normal"));
        glBindTextureUnit(2, gbuffer_fbo->get_color_attachment_handle_by_name("albedo"));
        glBindTextureUnit(3, gbuffer_fbo->get_color_attachment_handle_by_name("orm"));
        glBindTextureUnit(4, gbuffer_fbo->get_color_attachment_handle_by_name("emission"));
        glBindTextureUnit(5, _shadow_depth);
        glBindTextureUnit(6, _indirect_texture);
        glBindTextureUnit(7, _bloom_texture);
        render_quad();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer_fbo->get_handle());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, output_fbo->get_handle()); // write to output framebuffer
        glBlitFramebuffer(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height(), 0, 0,
                          Engine::get_window()->get_width(), Engine::get_window()->get_height(),
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }
}
