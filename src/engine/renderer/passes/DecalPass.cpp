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
        glCullFace(GL_FRONT);
        gbuffer->bind();
        shader->bind();
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
            if (const auto albedo = AssetManager::get_texture_by_name(decal_render_item.decal_component.albedo_name))
            {
                glBindTextureUnit(0, albedo->get_handle());
            }

            if (const auto normal = AssetManager::get_texture_by_name(decal_render_item.decal_component.normal_name))
            {
                glBindTextureUnit(1, normal->get_handle());
            }

            if (const auto orm = AssetManager::get_texture_by_name(decal_render_item.decal_component.orm_name))
            {
                glBindTextureUnit(2, orm->get_handle());
            }

            if (const auto emission =
                    AssetManager::get_texture_by_name(decal_render_item.decal_component.emission_name))
            {
                glBindTextureUnit(3, emission->get_handle());
            }
            render_cube();
        }
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
    }
}
