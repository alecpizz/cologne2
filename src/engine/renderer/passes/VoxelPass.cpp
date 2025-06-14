#include <engine/core/Engine.h>
#include <engine/renderer/OpenGLDebugScope.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>
#include <engine/editor/Editor.h>
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

        auto voxel_back_fbo = get_framebuffer_by_name("voxel_back");
        auto voxel_front_fbo = get_framebuffer_by_name("voxel_front");
        voxel_back_fbo->create("voxel cube back", Engine::get_window()->get_width(),
                               Engine::get_window()->get_height());
        voxel_back_fbo->create_attachment("color", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        voxel_front_fbo->create("voxel cube front", Engine::get_window()->get_width(),
                                Engine::get_window()->get_height());
        voxel_front_fbo->create_attachment("color", GL_RGBA16F, GL_NEAREST, GL_NEAREST);
        Engine::get_debug_ui()->add_image_entry("Voxel cube front",
                                                voxel_front_fbo->get_color_attachment_handle_by_name("color"),
                                                glm::vec2(Engine::get_window()->get_width(),
                                                          Engine::get_window()->get_height()));
        Engine::get_debug_ui()->add_image_entry("Voxel cube back",
                                                voxel_back_fbo->get_color_attachment_handle_by_name("color"),
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

        OpenGLDebugScope scope("Renderer::debug_voxel_pass");

        Shader *world_pos_shader = get_shader_by_name("world_pos_shader");
        world_pos_shader->bind();
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 model = glm::mat4(1.0f);
        world_pos_shader->set_mat4("model", (model));

        auto voxel_back_fbo = get_framebuffer_by_name("voxel_back");
        auto voxel_front_fbo = get_framebuffer_by_name("voxel_front");
        glCullFace(GL_FRONT);
        voxel_back_fbo->bind();
        voxel_back_fbo->set_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_cube();

        glCullFace(GL_BACK);
        voxel_front_fbo->bind();
        voxel_front_fbo->set_viewport();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_cube();

        auto voxelize_debug_shader = get_shader_by_name("voxelize_debug");
        voxelize_debug_shader->bind();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glBindTexture(GL_TEXTURE_3D, _voxel_texture);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTextureUnit(0, _voxel_texture);
        glBindTextureUnit(1, voxel_back_fbo->get_color_attachment_handle_by_name("color"));
        glBindTextureUnit(2, voxel_front_fbo->get_color_attachment_handle_by_name("color"));
        render_quad();

        glEnable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_3D, _voxel_texture);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_3D, 0);
    }


    void Renderer::voxelize_scene()
    {
        OpenGLDebugScope scope("Renderer::voxelize_scene");
        auto scene = Engine::get_scene();
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glViewport(0, 0, _voxel_data.voxel_dimensions, _voxel_data.voxel_dimensions);
        glClearTexImage(_voxel_texture, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(glm::vec4(0.0f)));

        auto shader = get_shader_by_name("voxelize");
        shader->bind();
        shader->set_mat4("lightSpaceMatrix", (_dir_light_space));
        shader->set_mat4("projection", (glm::ortho(-1.0f, 1.0f, -1.0f,
                                                   1.0f, -1.0f, 1.0f)));
        auto size = Engine::get_scene()->get_bounds().size();
        const float offset = 2.0f - 0.1f;
        glm::vec3 scale = glm::vec3(offset / fabs(size.x), offset / fabs(size.y), offset / fabs(size.z));
        shader->set_vec3("voxel_size", (scale));
        auto bounds = Engine::get_scene()->get_bounds();
        auto min = bounds.min;
        auto max = bounds.max;
        shader->set_vec3("grid_min", (min));
        shader->set_vec3("grid_max", (max));
        glBindImageTexture(6, _voxel_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindTextureUnit(8, get_framebuffer_by_name("dir_shadow")->get_depth_attachment_handle());
        // glBindTextureUnit(7, _shadow_depth);
        for (int idx = 0; idx < 3; idx++)
        {
            shader->set_int("render_axis", idx);
            for (auto &item: _render_items)
            {
                shader->set_mat4("model", item.transform.get_mat4());
                for (auto& mesh : item.model->get_meshes())
                {
                    item.model->get_materials()[mesh.get_material_index()].bind_all();
                    mesh.draw();
                }
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

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
    }
}
