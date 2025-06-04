//
// Created by alecpizz on 5/3/2025.
//
#include "engine/core/Engine.h"

#include "engine/renderer/DebugScope.h"
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
        fbo->release();
    }

    void Renderer::geometry_pass(Scene &scene)
    {
        DebugScope scope("Renderer::geometry_pass");

        auto fbo = get_framebuffer_by_name("gbuffer");
        fbo->bind();
        fbo->set_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto shader = get_shader_by_name("gbuffer");
        shader->bind();
        shader->set_mat4("projection", Engine::get_camera()->get_projection_matrix());
        shader->set_mat4("view", Engine::get_camera()->get_view_matrix());

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
        shader->set_mat4("projection", Engine::get_camera()->get_projection_matrix());
        shader->set_mat4("view", Engine::get_camera()->get_view_matrix());

        for (auto &[skinned_model, tr]: _skinned_render_items)
        {
            shader->set_mat4("model", tr.get_mat4());
            if (scene->get_animator_by_name(skinned_model->get_name()) != nullptr)
            {
                auto &animator = scene->get_animator_by_name(skinned_model->get_name());
                auto &bones = animator.get_bones();
                shader->set_mat4("bone_matrices", bones);
            }
            for (size_t j = 0; j < skinned_model->get_num_meshes(); j++)
            {
                auto &mesh = skinned_model->get_meshes()[j];
                Material &mat = skinned_model->get_materials()[mesh.get_material_index()];
                mat.bind_all();
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
        shader->set_mat4("projection", Engine::get_camera()->get_projection_matrix());
        shader->set_mat4("view", Engine::get_camera()->get_view_matrix());

        for (auto &particle: scene.get_particles())
        {
            particle.render();
        }

        fbo->release();
    }
}
