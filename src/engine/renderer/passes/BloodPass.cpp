//
// Created by alecpizz on 9/27/25.
//
#include <engine/asset_manager/AssetManager.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>

namespace cologne
{
    void Renderer::blood_pass()
    {
        if (_blood_render_items.empty())
        {
            return;
        }
        auto shader = get_shader_by_name("vat_blood");
        auto gbuffer = get_framebuffer_by_name("gbuffer");
        if (!shader || !gbuffer)
        {
            return;
        }

        gbuffer->bind();
        shader->bind();

        glBindVertexArray(get_vertex_data_vao());

        for (auto& render_item : _blood_render_items)
        {
            auto mesh = render_item.blood_component.mesh.get();
            if (!mesh)
            {
                continue;
            }

            auto pos = render_item.blood_component.position_texture.get();
            auto norm = render_item.blood_component.normal_texture.get();
            if (!pos || !norm)
            {
                continue;
            }

            glBindTextureUnit(0, pos->get_handle());
            glBindTextureUnit(1, norm->get_handle());
            shader->set_mat4("model", render_item.transform);
            shader->set_float("time", render_item.blood_component.time);
            shader->set_uint("entity_id", render_item.entity_id);
            shader->set_vec3("height_offset", render_item.blood_component.offset);

            glDrawElementsBaseVertex(GL_TRIANGLES, mesh->get_indices_count(), GL_UNSIGNED_INT,
                                   reinterpret_cast<void *>(sizeof(uint32_t) * mesh->get_first_index()),
                                   mesh->get_base_vertex());
        }

        glBindVertexArray(0);
    }

}
