//
// Created by alecpizz on 5/3/2025.
//
#include <engine/asset_manager/AssetManager.h>
#include <engine/renderer/types/SSBO.h>

#include "engine/core/Engine.h"

#include "engine/renderer/OpenGLDebugScope.h"
#include "engine/renderer/types/FrameBuffer.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/types/Shader.h"
#include <engine/editor/Editor.h>


namespace cologne
{
    void Renderer::init_gbuffer()
    {
        auto width = Engine::get_window()->get_width();
        auto height = Engine::get_window()->get_height();
        auto fbo = get_framebuffer_by_name("gbuffer");
        fbo->create("gbuffer", width, height);
        fbo->bind();
        fbo->create_attachment("position", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        fbo->create_attachment("normal", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        fbo->create_attachment("albedo", GL_RGBA8, GL_NEAREST, GL_NEAREST);
        fbo->create_attachment("orm", GL_RGB8, GL_NEAREST, GL_NEAREST);
        fbo->create_attachment("emission", GL_RGB8, GL_NEAREST, GL_NEAREST);
        fbo->create_attachment("entity_id", GL_R32UI, GL_NEAREST, GL_NEAREST);
        fbo->create_depth_attachment(GL_DEPTH_COMPONENT32F, GL_NEAREST, GL_NEAREST, GL_REPEAT);

        std::vector attachments = {"position", "normal", "albedo", "orm", "emission", "entity_id"};
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
        Engine::get_debug_ui()->add_image_entry("G_Entity_ID", fbo->get_color_attachment_handle_by_name("entity_id"),
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
        fbo->clear_attachment("position", 0.0f, 0.0f, 0.0f, 0.0f);
        fbo->clear_attachment("normal", 0.0f, 0.0f, 0.0f, 0.0f);
        fbo->clear_attachment("albedo", 0, 0, 0, 0);
        fbo->clear_attachment("orm", 0, 0, 0, 0);
        fbo->clear_attachment("emission", 0, 0, 0, 0);
        fbo->clear_attachment("entity_id", static_cast<uint32_t>(entt::null), entt::null, entt::null, entt::null);
        fbo->clear_depth_attachment();
        auto shader = get_shader_by_name("gbuffer");
        shader->bind();

        //frustum cull me NOW!
        get_ssbo_by_name("viewport")->bind(1);
        get_ssbo_by_name("lights")->bind(2);

        render_geometry();
        render_skinned_geometry();

        shader = get_shader_by_name("particle_render");
        shader->bind();
        shader->set_mat4("projection", get_camera_projection(_camera_transform, _cam));
        shader->set_mat4("view", get_camera_view(_camera_transform));
        shader->set_uint("entity_id", entt::null);

        //GET ME OUTTA HERE!
        for (auto &particle: Engine::get_scene()->get_particles())
        {
            particle.render();
        }



        fbo->release();
    }
}
