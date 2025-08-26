#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/editor/Editor.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>

namespace cologne
{
    std::vector<glm::ivec2> create_outline_offsets(int thickness = 1)
    {
        std::vector<glm::ivec2> result;
        for (int y = -thickness; y <= thickness; y++)
        {
            for (int x = -thickness; x <= thickness; x++)
            {
                //this stops the inside pixels from being included
                if (abs(x) == thickness || abs(y) == thickness)
                {
                    result.emplace_back(x, y);
                }
            }
        }
        return result;
    }

    void Renderer::init_outline()
    {
        auto fbo = get_framebuffer_by_name("outline");
        fbo->create("outline", Engine::get_render_target_width(), Engine::get_render_target_height());
        fbo->create_attachment("mask", GL_R8, GL_LINEAR, GL_LINEAR);
        fbo->create_attachment("result", GL_R8, GL_LINEAR, GL_LINEAR);
        Engine::get_editor()->add_image_entry("mask", fbo->get_color_attachment_handle_by_name("mask"),
                                                Engine::get_render_target_dimensions());
        Engine::get_editor()->add_image_entry("outline result", fbo->get_color_attachment_handle_by_name("result"),
                                                Engine::get_render_target_dimensions());
    }

    void Renderer::outline_pass()
    {
        //ez optimization here, if no render items, return!
        if (_outline_skinned_render_items.empty() && _outline_render_items.empty())
        {
            return;
        }
        auto gbuffer = get_framebuffer_by_name("gbuffer");
        auto outline_fbo = get_framebuffer_by_name("outline");
        auto output = get_framebuffer_by_name("output");
        if (!gbuffer || !outline_fbo)
        {
            LOG_ERROR("No G-Buffer or Outline Framebuffer!");
            return;
        }

        auto mask_shader = get_shader_by_name("outline_mask");
        auto outline_shader = get_shader_by_name("outline");
        auto composite = get_shader_by_name("outline_composite");
        if (!mask_shader || !outline_shader || !composite)
        {
            LOG_ERROR("no mask, outline or composite shader");
            return;
        }

        int outline_thickness = 2;
        static std::vector<glm::ivec2> offsets = create_outline_offsets(outline_thickness);

        outline_fbo->bind_depth_attachment(*gbuffer);
        outline_fbo->bind();
        outline_fbo->clear_attachment("mask", 0, 0, 0, 0);
        outline_fbo->clear_attachment("result", 0, 0, 0, 0);

        glDisable(GL_DEPTH_TEST);

        glDrawBuffer(outline_fbo->get_color_attachment_slot_by_name("mask"));
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        mask_shader->bind();
        //draw items into mask
        glBindVertexArray(get_vertex_data_vao());
        for (auto &outline_render_item: _outline_render_items)
        {
            mask_shader->set_mat4("model", outline_render_item.transform);
            const auto mesh = AssetManager::get_mesh_by_index(outline_render_item.mesh_idx);
            glDrawElementsBaseVertex(GL_TRIANGLES, mesh->get_indices_count(), GL_UNSIGNED_INT,
                                     reinterpret_cast<void *>(sizeof(uint32_t) * mesh->get_first_index()),
                                     mesh->get_base_vertex());
        }
        glBindVertexArray(0);
        glBindVertexArray(get_skinned_bind_pose_vao());
        for (auto &item: _outline_skinned_render_items)
        {
            mask_shader->set_mat4("model", item.transform);
            if (!item.bones.empty())
            {
                mask_shader->set_bool("is_skinned", true);
                mask_shader->set_mat4("bone_matrices", item.bones);
            }
            const auto mesh = AssetManager::get_skinned_mesh_by_index(item.mesh_idx);
            glDrawElementsBaseVertex(GL_TRIANGLES, mesh->get_indices_count(), GL_UNSIGNED_INT,
                                     reinterpret_cast<void *>(sizeof(uint32_t) * mesh->get_first_index()),
                                     mesh->get_base_vertex());
            mask_shader->set_bool("is_skinned", false);
        }
        glBindVertexArray(0);
        glEnable(GL_CULL_FACE);
        mask_shader->set_bool("is_skinned", false);
        //draw the outline a shit load
        outline_shader->bind();
        outline_shader->set_ivec2("offsets", offsets);
        size_t instance_count = offsets.size();
        outline_shader->set_int("offset_count", instance_count);
        glEnable(GL_BLEND);
        GLint blendSrc, blendDst;
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);
        glBlendEquation(GL_MAX);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawBuffer(outline_fbo->get_color_attachment_slot_by_name("result"));
        glBindImageTexture(0, outline_fbo->get_color_attachment_handle_by_name("mask"),
                           0, GL_FALSE, 0, GL_READ_ONLY,GL_R8);
        render_quad(instance_count);

        //composite
        glBindImageTexture(0, output->get_color_attachment_handle_by_name("color"),
                           0, GL_FALSE, 0, GL_READ_WRITE,GL_RGBA16F);
        glBindImageTexture(1, outline_fbo->get_color_attachment_handle_by_name("mask"),
                           0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
        glBindImageTexture(2, outline_fbo->get_color_attachment_handle_by_name("result"),
                           0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
        composite->bind();
        composite->dispatch(gbuffer->get_width() / 16, gbuffer->get_height() / 16, 1);

        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(blendSrc, blendDst);
        glEnable(GL_DEPTH_TEST);
    }
}
