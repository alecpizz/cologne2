//
// Created by alecpizz on 5/3/2025.
//
#include <engine/Engine.h>

#include "FrameBuffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "../Scene.h"

namespace cologne
{
    void Renderer::lit_pass()
    {
        auto shader = get_shader_by_name("lit");
        _output_fbo.bind();
        _output_fbo.set_viewport();
        // glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader->bind();
        shader->set_vec3("camera_pos", glm::value_ptr(Engine::get_camera()->get_position()));
        // env_irradiance.bind(IRRADIANCE_INDEX);
        // env_prefilter.bind(PREFILTER_INDEX);
        // env_brdf.bind(BRDF_INDEX);
        update_shadow(*shader);
        shader->set_mat4("view_inverse", glm::value_ptr(glm::inverse(Engine::get_camera()->get_view_matrix())));
        shader->set_mat4("view", glm::value_ptr(Engine::get_camera()->get_view_matrix()));
        shader->set_int("voxel_grid_size", _voxel_data.voxel_dimensions);
        shader->set_vec3("voxel_offset", glm::value_ptr(_voxel_data.voxel_offset));
        auto bounds = Engine::get_scene()->get_bounds();
        const glm::vec3 center = bounds.center();
        const glm::vec3 size = bounds.size(); //THIS IS WRONG!
        const float world_size = glm::max(size.x, glm::max(size.y, size.z));
        const float texelSize = 1.0f / _voxel_data.voxel_dimensions;
        const float voxel_size = world_size * texelSize;
        shader->set_float("voxel_size", voxel_size);
        auto min = bounds.min;
        auto max = bounds.max;
        shader->set_vec3("grid_min", glm::value_ptr(min));
        shader->set_vec3("grid_max", glm::value_ptr(max));
        shader->set_vec3("world_center", glm::value_ptr(center));
        shader->set_float("worldSizeHalf", 0.5f * world_size);

        shader->set_float("sampling_factor", 0.100f);
        shader->set_float("distance_offset", 3.9f);
        shader->set_float("max_distance", 2.0f);
        shader->set_bool("indirect_lighting_active", _apply_indirect_lighting);
        glBindTextureUnit(0, _gbuffer_fbo.get_color_attachment_handle_by_name("position"));
        glBindTextureUnit(1, _gbuffer_fbo.get_color_attachment_handle_by_name("normal"));
        glBindTextureUnit(2, _gbuffer_fbo.get_color_attachment_handle_by_name("albedo"));
        glBindTextureUnit(3, _gbuffer_fbo.get_color_attachment_handle_by_name("orm"));
        glBindTextureUnit(4, _gbuffer_fbo.get_color_attachment_handle_by_name("emission"));
        glBindTextureUnit(10, _gbuffer_fbo.get_color_attachment_handle_by_name("flat_normals"));
        glBindTextureUnit(5, _shadow_depth);
        glBindTextureUnit(9, _voxel_texture);
        render_quad();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _gbuffer_fbo.get_handle());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _output_fbo.get_handle()); // write to output framebuffer
        glBlitFramebuffer(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height(), 0, 0,
                          Engine::get_window()->get_width(), Engine::get_window()->get_height(),
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }
}
