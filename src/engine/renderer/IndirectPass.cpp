
#include <engine/Engine.h>

#include "DebugScope.h"
#include "FrameBuffer.h"
#include "Renderer.h"
#include "Shader.h"
namespace cologne
{
    void Renderer::init_indirect()
    {
        if (_indirect_texture != 0)
        {
            glDeleteTextures(1, &_indirect_texture);
        }
        glCreateTextures(GL_TEXTURE_2D, 1, &_indirect_texture);
        glTextureParameteri(_indirect_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_indirect_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_indirect_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(_indirect_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureStorage2D(_indirect_texture, 1, GL_RGBA8,
                           Engine::get_window()->get_width() / 2, Engine::get_window()->get_height() / 2);
        Engine::get_debug_ui()->add_image_entry("Indirect_Lighting", _indirect_texture,
            {Engine::get_window()->get_width() / 2, Engine::get_window()->get_height() / 2});
    }
    void Renderer::indirect_pass()
    {
        DebugScope scope("Renderer::indirect_pass");
        auto shader = get_shader_by_name("indirect");
        if (!shader)
        {
            return;
        }
        shader->bind();
        shader->set_int("voxel_grid_size", _voxel_data.voxel_dimensions);
        shader->set_vec3("voxel_offset", glm::value_ptr(_voxel_data.voxel_offset));
        auto bounds = Engine::get_scene()->get_bounds();
        const glm::vec3 center = bounds.center();
        const glm::vec3 size = bounds.size();
        const float world_size = glm::max(size.x, glm::max(size.y, size.z));
        const float texelSize = 1.0f / _voxel_data.voxel_dimensions;
        const float voxel_size = world_size * texelSize;
        shader->set_float("voxel_size", voxel_size);
        auto min = bounds.min;
        auto max = bounds.max;
        shader->set_vec3("grid_min", glm::value_ptr(min));
        shader->set_vec3("grid_max", glm::value_ptr(max));

        shader->set_bool("indirect_lighting_active", _apply_indirect_lighting);
        glBindImageTexture(0, _indirect_texture, 0, GL_FALSE, 0,
            GL_WRITE_ONLY, GL_RGBA8);
        glBindTextureUnit(1, _voxel_texture);
        glBindTextureUnit(2, _gbuffer_fbo.get_color_attachment_handle_by_name("position"));
        glBindTextureUnit(3, _gbuffer_fbo.get_color_attachment_handle_by_name("normal"));

        const uint32_t work_group_size = 16;
        uint32_t width = Engine::get_window()->get_width() / 2;
        uint32_t height = Engine::get_window()->get_height() / 2;
        uint32_t num_x = (width + work_group_size - 1) / work_group_size;
        uint32_t num_y = (height + work_group_size - 1) / work_group_size;
        shader->dispatch(num_x, num_y, 1);
        shader->wait();
    }
}