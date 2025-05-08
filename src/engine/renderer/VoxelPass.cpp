#include <engine/Engine.h>

#include "Renderer.h"
#include "Shader.h"
//
// Created by alecpizz on 5/4/2025.
//
namespace cologne
{
    void Renderer::init_voxels()
    {
        glGenTextures(1, &_voxel_texture);
        glBindTexture(GL_TEXTURE_3D, _voxel_texture);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        // glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, voxel_size, voxel_size, voxel_size);
        int numVoxels = _voxel_data.voxel_dimensions * _voxel_data.voxel_dimensions * _voxel_data.voxel_dimensions;
        auto *data = new GLfloat[4 * numVoxels];
        memset(data, 0.0f, 4 * numVoxels);
        // glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, voxel_size, voxel_size, voxel_size, 0, GL_RGBA, GL_FLOAT, data);
        glTexStorage3D(GL_TEXTURE_3D, log2(_voxel_data.voxel_dimensions),
                       GL_RGBA16F, _voxel_data.voxel_dimensions, _voxel_data.voxel_dimensions,
                       _voxel_data.voxel_dimensions);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
                        _voxel_data.voxel_dimensions, _voxel_data.voxel_dimensions,
                        _voxel_data.voxel_dimensions, GL_RGBA,
                        GL_FLOAT, data);
        glGenerateMipmap(GL_TEXTURE_3D);
        glBindTexture(GL_TEXTURE_3D, 0);
        delete[] data;

        _voxel_back_fbo.create("voxel cube back", Engine::get_window()->get_width(),
                               Engine::get_window()->get_height());
        _voxel_back_fbo.create_attachment("color", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        _voxel_front_fbo.create("voxel cube front", Engine::get_window()->get_width(),
                                Engine::get_window()->get_height());
        _voxel_front_fbo.create_attachment("color", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        _voxel_fbo.create("voxel", _voxel_data.voxel_dimensions, _voxel_data.voxel_dimensions);
        _voxel_fbo.set_empty();
        Engine::get_debug_ui()->add_image_entry("Voxel cube front",
                                                _voxel_front_fbo.get_color_attachment_handle_by_name("color"),
                                                glm::vec2(Engine::get_window()->get_width(),
                                                          Engine::get_window()->get_height()));
        Engine::get_debug_ui()->add_image_entry("Voxel cube back",
                                                _voxel_back_fbo.get_color_attachment_handle_by_name("color"),
                                                glm::vec2(Engine::get_window()->get_width(),
                                                          Engine::get_window()->get_height()));
        Engine::get_debug_ui()->add_button("Voxelize Scene", [&]()
        {
            voxelize_scene();
        });
    }

    void Renderer::debug_voxel_pass()
    {
        if (!_voxel_debug_visuals)
        {
            return;
        }

        auto world_pos_shader = get_shader_by_name("world_pos_shader");
        world_pos_shader->bind();
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        world_pos_shader->set_mat4("projection", glm::value_ptr(Engine::get_camera()->get_projection_matrix()));
        world_pos_shader->set_mat4("view", glm::value_ptr(Engine::get_camera()->get_view_matrix()));
        glm::mat4 model = glm::mat4(1.0f);
        world_pos_shader->set_mat4("model", glm::value_ptr(model));
        world_pos_shader->set_vec3("camera_position", glm::value_ptr(Engine::get_camera()->get_position()));

        glCullFace(GL_FRONT);
        _voxel_back_fbo.bind();
        _voxel_back_fbo.set_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_cube();

        glCullFace(GL_BACK);
        _voxel_front_fbo.bind();
        _voxel_front_fbo.set_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_cube();

        auto voxelize_debug_shader = get_shader_by_name("voxelize_debug");
        voxelize_debug_shader->bind();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        voxelize_debug_shader->set_mat4("projection",
                                        glm::value_ptr(Engine::get_camera()->get_projection_matrix()));
        voxelize_debug_shader->set_mat4("view", glm::value_ptr(Engine::get_camera()->get_view_matrix()));
        voxelize_debug_shader->set_vec3("camera_position", glm::value_ptr(Engine::get_camera()->get_position()));

        glBindTextureUnit(0, _voxel_texture);
        glBindTextureUnit(1, _voxel_back_fbo.get_color_attachment_handle_by_name("color"));
        glBindTextureUnit(2, _voxel_front_fbo.get_color_attachment_handle_by_name("color"));
        render_quad();

        glEnable(GL_DEPTH_TEST);
    }


    void Renderer::voxelize_scene()
    {
        auto scene = Engine::get_scene();
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        _voxel_fbo.bind();
        _voxel_fbo.set_viewport();
        glClearTexImage(_voxel_texture, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(glm::vec4(0.0f)));
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        auto shader = get_shader_by_name("voxelize");
        shader->bind();
        update_lights(*shader);
        shader->set_mat4("projection", glm::value_ptr(glm::ortho(-1.0f, 1.0f, -1.0f,
                                                                          1.0f, -1.0f, 1.0f)));
        auto size = Engine::get_scene()->get_bounds().size();
        const float offset = 2.0f - 0.1f;
        glm::vec3 scale = glm::vec3(offset / fabs(size.x), offset / fabs(size.y), offset / fabs(size.z));
        shader->set_vec3("voxel_size", glm::value_ptr(scale));
        auto bounds = Engine::get_scene()->get_bounds();
        auto min = bounds.min;
        auto max = bounds.max;
        shader->set_vec3("grid_min", glm::value_ptr(min));
        shader->set_vec3("grid_max", glm::value_ptr(max));
        glBindImageTexture(6, _voxel_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
        // glBindTextureUnit(7, _shadow_depth);
        for (size_t i = 0; i < scene->get_model_count(); i++)
        {
            auto model = scene->get_model_by_index(i);
            shader->set_mat4("model",
                                      glm::value_ptr(
                                          model->get_transform()->get_model_matrix()));
            for (size_t j = 0; j < model->get_num_meshes(); j++)
            {
                auto &mesh = scene->get_model_by_index(i)->get_meshes()[j];
                model->get_materials()[mesh.get_material_index()].bind_all();
                mesh.draw();
            }
        }

        auto mipmap_shader = get_shader_by_name("mipmap");
        mipmap_shader->bind();
        int current_height = _voxel_data.voxel_dimensions;
        int current_width = _voxel_data.voxel_dimensions;
        int current_depth = _voxel_data.voxel_dimensions;
        for (int mip = 0; mip < 7; mip++)
        {
            int next = mip + 1;
            int dest_width = glm::max(1, current_height >> 1);
            int dest_height = glm::max(1, current_width >> 1);
            int dest_depth = glm::max(1, current_depth >> 1);


            glBindImageTexture(0,
                               _voxel_texture,
                               next,
                               GL_TRUE,
                               0, GL_WRITE_ONLY, GL_RGBA16F);
            glBindImageTexture(1, _voxel_texture, mip, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
            int localSize = 8;
            uint32_t x = (dest_width + localSize - 1) / localSize;
            uint32_t y = (dest_height + localSize - 1) / localSize;
            uint32_t z = (dest_depth + localSize - 1) / localSize;
            mipmap_shader->dispatch(x, y, z);
            mipmap_shader->wait();
            current_depth = dest_depth;
            current_height = dest_height;
            current_width = dest_width;
        }

        glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
