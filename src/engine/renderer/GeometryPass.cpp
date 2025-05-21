//
// Created by alecpizz on 5/3/2025.
//
#include <engine/Engine.h>

#include "DebugScope.h"
#include "FrameBuffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "../Scene.h"
namespace cologne
{
    void Renderer::init_gbuffer()
    {
        auto width = Engine::get_window()->get_width();
        auto height = Engine::get_window()->get_height();
        if (_gbuffer_fbo.is_valid())
        {
            _gbuffer_fbo.resize( width, height);
            return;
        }
        _gbuffer_fbo.create("GBuffer", width, height);
        _gbuffer_fbo.bind();
        _gbuffer_fbo.create_attachment("position", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        _gbuffer_fbo.create_attachment("normal", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        _gbuffer_fbo.create_attachment("albedo", GL_RGBA8, GL_NEAREST, GL_NEAREST);
        _gbuffer_fbo.create_attachment("orm", GL_RGB8, GL_NEAREST, GL_NEAREST);
        _gbuffer_fbo.create_attachment("emission", GL_RGB8, GL_NEAREST, GL_NEAREST);


        _gbuffer_fbo.create_depth_attachment(GL_DEPTH_COMPONENT32F, GL_NEAREST, GL_NEAREST, GL_REPEAT);
        std::vector attachments = {"position", "normal", "albedo", "orm", "emission"};
        _gbuffer_fbo.draw_buffers(attachments.data(), static_cast<uint32_t>(attachments.size()));
        _gbuffer_fbo.release();

        _output_fbo.create("output", width, height);
        _output_fbo.create_attachment("color", GL_RGBA16F, GL_LINEAR, GL_LINEAR);
        _output_fbo.create_depth_attachment(GL_DEPTH_COMPONENT32F, GL_NEAREST, GL_NEAREST, GL_REPEAT);
        _output_fbo.release();

        Engine::get_debug_ui()->add_image_entry("G_Normals", _gbuffer_fbo.get_color_attachment_handle_by_name("normal"),
            glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_Position",
            _gbuffer_fbo.get_color_attachment_handle_by_name("position"), glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_Albedo", _gbuffer_fbo.get_color_attachment_handle_by_name("albedo"),
            glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_ORM", _gbuffer_fbo.get_color_attachment_handle_by_name("orm"),
            glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_Depth", _gbuffer_fbo.get_depth_attachment_handle(),
            glm::vec2(width, height));
        Engine::get_debug_ui()->add_image_entry("G_Emission", _gbuffer_fbo.get_color_attachment_handle_by_name("emission"),
            glm::vec2(width, height));
    }

    void Renderer::geometry_pass(Scene &scene)
    {
        DebugScope scope("Renderer::geometry_pass");
        _gbuffer_fbo.bind();
        _gbuffer_fbo.set_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto shader = get_shader_by_name("gbuffer");
        shader->bind();
        shader->set_mat4("projection", &Engine::get_camera()->get_projection_matrix()[0][0]);
        shader->set_mat4("view", &Engine::get_camera()->get_view_matrix()[0][0]);

        for (size_t i = 0; i < scene.get_model_count(); i++)
        {
            auto model = scene.get_model_by_index(i);
            if (!model->get_active())
            {
                continue;
            }
            if (model->get_gi_only())
            {
                continue;
            }
            shader->set_mat4("model", glm::value_ptr(model->get_transform()->get_model_matrix()));
            for (size_t j = 0; j < model->get_num_meshes(); j++)
            {
                Mesh mesh = model->get_meshes()[j];
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

        _gbuffer_fbo.release();
    }
}