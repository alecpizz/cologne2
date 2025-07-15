#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/renderer/OpenGLDebugScope.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>
#include <engine/editor/Editor.h>
#include <engine/renderer/types/Light.h>
#include <engine/renderer/types/SSBO.h>
//
// Created by alecpizz on 5/4/2025.
//
namespace cologne
{
    void Renderer::init_voxels()
    {
        //color
        glGenTextures(1, &_voxel_texture_color);
        glBindTexture(GL_TEXTURE_3D, _voxel_texture_color);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        int numVoxels = _voxel_data.voxel_dimensions * _voxel_data.voxel_dimensions * _voxel_data.voxel_dimensions;
        auto *data = new GLfloat[4 * numVoxels];
        memset(data, 0.0f, 4 * numVoxels);
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
        // auto *norm_data = new GLuint[4 * numVoxels];
        // memset(norm_data, 0, 4 * numVoxels);
        //normals
        glGenTextures(1, &_voxel_texture_normal);
        glBindTexture(GL_TEXTURE_3D, _voxel_texture_normal);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        // glTexStorage3D(GL_TEXTURE_3D, 1,
        //                GL_R16UI, _voxel_data.voxel_dimensions, _voxel_data.voxel_dimensions,
        //                _voxel_data.voxel_dimensions);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, _voxel_data.voxel_dimensions, _voxel_data.voxel_dimensions,
                     _voxel_data.voxel_dimensions, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
        // glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
        //                 _voxel_data.voxel_dimensions, _voxel_data.voxel_dimensions,
        //                 _voxel_data.voxel_dimensions, GL_RED,
        //                 GL_UNSIGNED_INT, norm_data);
        glBindTexture(GL_TEXTURE_3D, 0);

        // delete[] norm_data;
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

        auto voxelize_debug_shader = get_shader_by_name("voxelize_debug");
        voxelize_debug_shader->bind();
        voxelize_debug_shader->set_float("step_multiplier", 0.4f);
        voxelize_debug_shader->set_float("cone_angle", 0.5f);
        auto bounds = Engine::get_scene()->get_bounds();
        auto min = bounds.min;
        auto max = bounds.max;
        voxelize_debug_shader->set_vec3("grid_min", min);
        voxelize_debug_shader->set_vec3("grid_max", max);
        glBindTexture(GL_TEXTURE_3D, _voxel_texture_color);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        auto fbo = get_framebuffer_by_name("output");
        glBindImageTexture(0, fbo->get_color_attachment_handle_by_name("color"),
            0,GL_FALSE, 0, GL_WRITE_ONLY,GL_RGBA16F);
        glBindTextureUnit(0, _voxel_texture_color);
        glBindTextureUnit(1, _skybox_texture);

        const uint32_t work_group_size = 8;
        uint32_t width = Engine::get_window()->get_width();
        uint32_t height = Engine::get_window()->get_height();
        uint32_t num_x = (width + work_group_size - 1) / work_group_size;
        uint32_t num_y = (height + work_group_size - 1) / work_group_size;
        voxelize_debug_shader->dispatch(num_x, num_y, 1);
        voxelize_debug_shader->wait(GL_TEXTURE_FETCH_BARRIER_BIT);
        glBindTexture(GL_TEXTURE_3D, _voxel_texture_color);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_3D, 0);
    }


    void Renderer::voxelize_scene()
    {
        OpenGLDebugScope scope("Renderer::voxelize_scene");
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        // glDisable(GL_BLEND);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        glViewport(0, 0, _voxel_data.voxel_dimensions, _voxel_data.voxel_dimensions);
        glClearTexImage(_voxel_texture_color, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(glm::vec4(0.0f)));
        glClearTexImage(_voxel_texture_normal, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, glm::value_ptr(glm::uvec4(0)));
        auto cull_shader = get_shader_by_name("frustum_culling");
        cull_shader->bind();
        cull_shader->set_int("non_cull_amount", 1);
        get_ssbo_by_name("draw_cmds")->bind(6);
        const int work_group_size = 64;
        cull_shader->dispatch((_render_items.size() + work_group_size - 1) / work_group_size, 1, 1);
        cull_shader->wait(GL_SHADER_STORAGE_BARRIER_BIT);
        get_ssbo_by_name("skinned_draw_cmds")->bind(6);
        cull_shader->dispatch((_render_items.size() + work_group_size - 1) / work_group_size, 1, 1);
        cull_shader->wait(GL_SHADER_STORAGE_BARRIER_BIT);


        auto shader = get_shader_by_name("voxelize");
        shader->bind();
        shader->set_int("num_lights", _lights.size());
        auto size = Engine::get_scene()->get_bounds().size();
        const float offset = 2.0f - 0.1f;
        glm::vec3 scale = glm::vec3(offset / fabs(size.x), offset / fabs(size.y), offset / fabs(size.z));
        shader->set_vec3("voxel_size", (scale));
        auto bounds = Engine::get_scene()->get_bounds();
        auto min = bounds.min;
        auto max = bounds.max;
        shader->set_vec3("grid_min", (min));
        shader->set_vec3("grid_max", (max));

        glBindImageTexture(6, _voxel_texture_color, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindImageTexture(7, _voxel_texture_normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        get_ssbo_by_name("lights")->bind(2);
        for (int idx = 0; idx < 3; idx++)
        {
            shader->set_int("render_axis", idx);
            render_geometry();
            render_skinned_geometry();
        }
        //
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get_skinned_bind_pose_ebo());
        // glBindBuffer(GL_DRAW_INDIRECT_BUFFER, get_ssbo_by_name("skinned_draw_cmds")->get_handle());
        // glBindVertexArray(get_skinned_vao());
        // for (int idx = 0; idx < 3; idx++)
        // {
        //     shader->set_int("render_axis", idx);
        //     glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, _skinned_render_items.size(), 0);
        // }
        //


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
                               _voxel_texture_color,
                               next,
                               GL_TRUE,
                               0, GL_WRITE_ONLY, GL_RGBA16F);
            glBindImageTexture(1, _voxel_texture_color, mip, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
            glBindImageTexture(2, _voxel_texture_normal, next, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
            glBindImageTexture(3, _voxel_texture_normal, mip, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
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

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
    }
}
