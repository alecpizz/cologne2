
#include <engine/Engine.h>

#include "DebugScope.h"
#include "FrameBuffer.h"
#include "Renderer.h"
#include "Shader.h"
namespace cologne
{
    const uint32_t pixel_ratio = 2;
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
                           Engine::get_window()->get_width() / pixel_ratio, Engine::get_window()->get_height() / pixel_ratio);
        Engine::get_debug_ui()->add_image_entry("Indirect_Lighting", _indirect_texture,
            {Engine::get_window()->get_width() / pixel_ratio, Engine::get_window()->get_height() / pixel_ratio});
    }
    void Renderer::indirect_pass()
    {
        if (!_apply_indirect_lighting)
        {
            return;
        }
        DebugScope scope("Renderer::indirect_pass");
        auto shader = get_shader_by_name("indirect");
        if (!shader)
        {
            return;
        }
        shader->bind();
        auto bounds = Engine::get_scene()->get_bounds();
        const glm::vec3 size = bounds.size();
        const float world_size = glm::max(size.x, glm::max(size.y, size.z));
        const float texelSize = 1.0f / _voxel_data.voxel_dimensions;
        const float voxel_size = world_size * texelSize;
        auto min = bounds.min;
        auto max = bounds.max;
        shader->set_vec3("grid_min", glm::value_ptr(min));
        shader->set_vec3("grid_max", glm::value_ptr(max));
        shader->set_float("voxel_size", voxel_size);
        shader->set_vec3("voxel_offset", glm::value_ptr(_voxel_data.voxel_offset));
        shader->set_int("voxel_grid_size", _voxel_data.voxel_dimensions);
        
        glBindImageTexture(0, _indirect_texture, 0, GL_FALSE, 0,
            GL_WRITE_ONLY, GL_RGBA8);
        glBindTextureUnit(1, _voxel_texture);
        glBindTextureUnit(2, _gbuffer_fbo.get_color_attachment_handle_by_name("position"));
        glBindTextureUnit(3, _gbuffer_fbo.get_color_attachment_handle_by_name("normal"));

        const uint32_t work_group_size = 16;
        uint32_t width = Engine::get_window()->get_width() / pixel_ratio;
        uint32_t height = Engine::get_window()->get_height() / pixel_ratio;
        uint32_t num_x = (width + work_group_size - 1) / work_group_size;
        uint32_t num_y = (height + work_group_size - 1) / work_group_size;
        shader->dispatch(num_x, num_y, 1);
        shader->wait();
    }
}