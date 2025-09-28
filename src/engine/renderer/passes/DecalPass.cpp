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
        for (const auto &decal_render_item: _decal_render_items)
        {
            //draw a cube
            shader->set_mat4("model", decal_render_item.transform);
            shader->set_mat4("model_inverse", glm::inverse(decal_render_item.transform.transform));
            if (const auto albedo = AssetManager::get_texture_by_name(decal_render_item.decal_component.albedo_name))
            {
                glBindTextureUnit(albedo->get_handle(), 0);
            }

            if (const auto normal = AssetManager::get_texture_by_name(decal_render_item.decal_component.normal_name))
            {
                glBindTextureUnit(normal->get_handle(), 1);
            }

            if (const auto orm = AssetManager::get_texture_by_name(decal_render_item.decal_component.orm_name))
            {
                glBindTextureUnit(orm->get_handle(), 2);
            }

            if (const auto emission =
                    AssetManager::get_texture_by_name(decal_render_item.decal_component.emission_name))
            {
                glBindTextureUnit(emission->get_handle(), 3);
            }
            render_cube();
        }
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
    }
}
