//
// Created by alecpizz on 5/3/2025.
//
#include <engine/renderer/types/SSBO.h>

#include "engine/core/Engine.h"

#include "engine/renderer/OpenGLDebugScope.h"
#include "engine/renderer/types/FrameBuffer.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/types/Shader.h"

namespace cologne
{
    void Renderer::init_gbuffer()
    {
        auto width = Engine::get_window()->get_width();
        auto height = Engine::get_window()->get_height();
        auto fbo = get_framebuffer_by_name("gbuffer");
        if (fbo->is_valid())
        {
            fbo->resize(width, height);
            return;
        }
        fbo->create("gbuffer", width, height);
        fbo->bind();
        fbo->create_attachment("position", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        fbo->create_attachment("normal", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        fbo->create_attachment("albedo", GL_RGBA8, GL_NEAREST, GL_NEAREST);
        fbo->create_attachment("orm", GL_RGB8, GL_NEAREST, GL_NEAREST);
        fbo->create_attachment("emission", GL_RGB8, GL_NEAREST, GL_NEAREST);


        fbo->create_depth_attachment(GL_DEPTH_COMPONENT32F, GL_NEAREST, GL_NEAREST, GL_REPEAT);
        std::vector attachments = {"position", "normal", "albedo", "orm", "emission"};
        fbo->draw_buffers(attachments.data(), static_cast<uint32_t>(attachments.size()));

        Engine::get_debug_ui()->add_image_entry("G_Normals", fbo->get_color_attachment_handle_by_name("normal"),
                                                glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_Position",
                                                fbo->get_color_attachment_handle_by_name("position"),
                                                glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_Albedo", fbo->get_color_attachment_handle_by_name("albedo"),
                                                glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_ORM", fbo->get_color_attachment_handle_by_name("orm"),
                                                glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_Depth", fbo->get_depth_attachment_handle(),
                                                glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_Emission", fbo->get_color_attachment_handle_by_name("emission"),
                                                glm::vec2(width, height));

        fbo->release();

        fbo = get_framebuffer_by_name("output");
        fbo->create("output", width, height);
        fbo->create_attachment("color", GL_RGBA16F, GL_LINEAR, GL_LINEAR);
        fbo->create_depth_attachment(GL_DEPTH_COMPONENT32F, GL_NEAREST, GL_NEAREST, GL_REPEAT);
        Engine::get_debug_ui()->add_image_entry("output", fbo->get_color_attachment_handle_by_name("color"),
                                                glm::vec2(width, height));
        fbo->release();
    }

    void Renderer::geometry_pass()
    {
        OpenGLDebugScope scope("Renderer::geometry_pass");

        auto fbo = get_framebuffer_by_name("gbuffer");
        fbo->bind();
        fbo->set_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto shader = get_shader_by_name("gbuffer");
        shader->bind();

        for (auto &[model, tr, gi]: _render_items)
        {
            if (gi)
            {
                continue;
            }
            shader->set_mat4("model", tr.get_mat4());
            for (auto &mesh: model->get_meshes())
            {
                Material mat = model->get_materials()[mesh.get_material_index()];
                mat.bind_all();
                mesh.draw();
                if (mat.metallic.get_handle() == 0)
                {
                    shader->set_float("metallic", mat.metallic_override);
                }

                if (mat.roughness.get_handle() == 0)
                {
                    shader->set_float("roughness", mat.metallic_override);
                }

                glBindTextureUnit(ALBEDO_INDEX, 0);
                glBindTextureUnit(AO_INDEX, 0);
                glBindTextureUnit(METALLIC_INDEX, 0);
                glBindTextureUnit(ROUGHNESS_INDEX, 0);
                glBindTextureUnit(NORMAL_INDEX, 0);
                glBindTextureUnit(EMISSION_INDEX, 0);
            }
        }

        shader = get_shader_by_name("skinned_gbuffer");
        shader->bind();

        for (auto &[skinned_model, tr, bones]: _skinned_render_items)
        {
            shader->set_mat4("model", tr.get_mat4());
            if (!bones.empty())
            {
                shader->set_mat4("bone_matrices", bones);
            }
            else
            {
                static std::vector<glm::mat4> empty_bones (200, glm::mat4(1.0f));
                shader->set_mat4("bone_matrices", empty_bones);
            }
            for (size_t j = 0; j < skinned_model->get_num_meshes(); j++)
            {
                auto &mesh = skinned_model->get_meshes()[j];
                Material &mat = skinned_model->get_materials()[mesh.get_material_index()];
                mat.bind_all();
                if (mat.metallic.get_handle() == 0)
                {
                    shader->set_float("metallic", mat.metallic_override);
                }

                if (mat.roughness.get_handle() == 0)
                {
                    shader->set_float("roughness", mat.metallic_override);
                }
                mesh.draw();
                glBindTextureUnit(ALBEDO_INDEX, 0);
                glBindTextureUnit(AO_INDEX, 0);
                glBindTextureUnit(METALLIC_INDEX, 0);
                glBindTextureUnit(ROUGHNESS_INDEX, 0);
                glBindTextureUnit(NORMAL_INDEX, 0);
                glBindTextureUnit(EMISSION_INDEX, 0);
            }
        }

        shader = get_shader_by_name("particle_render");
        shader->bind();
        shader->set_mat4("projection", get_camera_projection(_camera_transform, _cam));
        shader->set_mat4("view", get_camera_view(_camera_transform));

        //GET ME OUTTA HERE!
        for (auto &particle: Engine::get_scene()->get_particles())
        {
            particle.render();
        }

        fbo->release();
    }
}
