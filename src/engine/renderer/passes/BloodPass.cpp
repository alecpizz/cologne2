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
            auto mesh = AssetManager::get_mesh_by_name(render_item.blood_component.mesh_name);
            if (!mesh)
            {
                continue;
            }

            shader->set_mat4("model", render_item.transform);

            glDrawElementsBaseVertex(GL_TRIANGLES, mesh->get_indices_count(), GL_UNSIGNED_INT,
                                   reinterpret_cast<void *>(sizeof(uint32_t) * mesh->get_first_index()),
                                   mesh->get_base_vertex());
        }

        glBindVertexArray(0);
    }

}
