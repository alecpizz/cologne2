#include <engine/asset_manager/AssetManager.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>
//
// Created by alecpizz on 9/28/25.
//
namespace cologne
{
    void Renderer::decal_pass()
    {
        if (_decal_render_items.empty())
        {
            return;
        }
        auto gbuffer = get_framebuffer_by_name("gbuffer");
        auto shader = get_shader_by_name("screenspace_decal");
        if (!gbuffer || !shader)
        {
            return;
        }

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glCullFace(GL_FRONT);
        gbuffer->bind();
        shader->bind();
        glEnable(GL_BLEND);
        glBlendFunci(2, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationi(2, GL_FUNC_ADD);
        glBlendFunci(1, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationi(1, GL_FUNC_ADD);
        glBlendFunci(3, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationi(3, GL_FUNC_ADD);

        glBindTextureUnit(5, gbuffer->get_depth_attachment_handle());
        glBindTextureUnit(6, gbuffer->get_color_attachment_handle_by_name("position"));
        glBindTextureUnit(7, gbuffer->get_color_attachment_handle_by_name("albedo"));
        glBindTextureUnit(8, gbuffer->get_color_attachment_handle_by_name("normal"));
        glBindTextureUnit(4, gbuffer->get_color_attachment_handle_by_name("orm"));
        for (const auto &decal_render_item: _decal_render_items)
        {
            //draw a cube
            shader->set_vec4("tint_color", decal_render_item.decal_component.color_tint);
            shader->set_mat4("model", decal_render_item.transform);
            shader->set_mat4("model_inverse", glm::inverse(decal_render_item.transform.transform));
            shader->set_mat4("model_normal", glm::transpose(glm::inverse(decal_render_item.transform.transform)));
            if (const auto albedo = decal_render_item.decal_component.albedo.get())
            {
                glBindTextureUnit(0, albedo->get_handle());
            }

            if (const auto normal = decal_render_item.decal_component.normal.get())
            {
                glBindTextureUnit(1, normal->get_handle());
            }

            if (const auto orm = decal_render_item.decal_component.orm.get())
            {
                glBindTextureUnit(2, orm->get_handle());
            }

            if (const auto emission = decal_render_item.decal_component.emission.get())
            {
                glBindTextureUnit(3, emission->get_handle());
            }
            render_cube();
        }
        glBlendFunci(2, GL_ONE, GL_ZERO);
        glBlendEquationi(2, GL_FUNC_ADD);
        glBlendFunci(1, GL_ONE, GL_ZERO);
        glBlendEquationi(1, GL_FUNC_ADD);
        glBlendFunci(3, GL_ONE, GL_ZERO);
        glBlendEquationi(3, GL_FUNC_ADD);
        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
    }
}
