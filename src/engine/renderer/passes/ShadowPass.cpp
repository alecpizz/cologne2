#include <engine/core/Engine.h>
#include <engine/renderer/OpenGLDebugScope.h>
#include <engine/renderer/types/Shader.h>
#include <engine/editor/Editor.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Light.h>
#include <engine/renderer/types/SSBO.h>

namespace cologne
{
    uint32_t point_shadow_fbo = 0;
    uint32_t dir_shadow_size = 1024;
    float shadow_near = 0.1f;
    float shadow_far = 1200.0f;
    float point_shadow_near = 1.0f;
    glm::mat4 _cam_view;
    glm::mat4 _cam_proj;

    uint32_t create_point_shadow_texture();

    uint32_t create_point_shadow_texture()
    {
        uint32_t handle_result = 0;
        glGenTextures(1, &handle_result);
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle_result);
        for (uint32_t i = 0; i < 6; i++)
        {
            constexpr int32_t size = 512;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT16,
                         size, size, 0, GL_DEPTH_COMPONENT,
                         GL_FLOAT, nullptr);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        return handle_result;
    }

    uint32_t create_dir_shadow_texture(int32_t size)
    {
        uint32_t handle_result = 0;
        glGenTextures(1, &handle_result);
        glBindTexture(GL_TEXTURE_2D, handle_result);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, size,
                     size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        return handle_result;
    }

    std::vector<glm::mat4> create_shadow_projection_matrices(glm::vec3 position, float far)
    {
        std::vector<glm::mat4> shadowTransforms;
        glm::mat4 proj = glm::perspective(glm::radians(90.0f),
                                          1.0f, point_shadow_near, far);
        shadowTransforms.push_back(proj *
                                   glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0),
                                               glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(proj *
                                   glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0),
                                               glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(proj *
                                   glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0),
                                               glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.push_back(proj *
                                   glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0),
                                               glm::vec3(0.0, 0.0, -1.0)));
        shadowTransforms.push_back(proj *
                                   glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0),
                                               glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(proj *
                                   glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0),
                                               glm::vec3(0.0, -1.0, 0.0)));
        return shadowTransforms;
    }

    void Renderer::init_shadow()
    {
        auto texture = Texture(create_dir_shadow_texture(4096), 4096, 4096, 1);
        texture.make_resident();
        _dir_shadow_maps[texture.get_bindless_handle()] = texture;
        _dir_shadow_queue.emplace(texture);
        glGenFramebuffers(1, &point_shadow_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, point_shadow_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                             _dir_shadow_maps[texture.get_bindless_handle()].get_handle(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Engine::get_debug_ui()->add_float_entry("shadow near_plane", shadow_near);
        Engine::get_debug_ui()->add_float_entry("shadow far_plane", shadow_far);
    }

    void Renderer::dir_shadow_pass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, point_shadow_fbo);
        OpenGLDebugScope scope("Renderer::dir_shadow_pass");
        auto shadow_shader = get_shader_by_name("dir_shadow");

        for (auto &pair: _lights)
        {
            auto& light = pair.second.light;
            if (light.type != Directional && light.type != Spot)
            {
                continue;
            }
            if (pair.second.light.active == 0)
            {
                continue;
            }
            if (!pair.second.cast_shadows)
            {
                continue;
            }
            if (!pair.second.dirty)
            {
                continue;
            }

            pair.second.dirty = false;

            if (light.shadow_handle == 0)
            {
                //get a new handle!
                if (!_dir_shadow_queue.empty())
                {
                    //empty list of ddir shadows, make new one
                    auto texture = Texture(create_dir_shadow_texture(1024), 1024, 1024, 1);
                    texture.make_resident();
                    _dir_shadow_maps[texture.get_bindless_handle()] = texture;
                    _dir_shadow_queue.emplace(texture);
                }
                light.shadow_handle = _dir_shadow_queue.front().get_bindless_handle();
                _dir_shadow_queue.pop();
            }

            glViewport(0, 0, _dir_shadow_maps[light.shadow_handle].get_width(),
                       _dir_shadow_maps[light.shadow_handle].get_height());
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                 _dir_shadow_maps[light.shadow_handle].get_handle(), 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glClear(GL_DEPTH_BUFFER_BIT);
            glm::mat4 light_projection;
            if (light.type == Directional)
            {
                light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, shadow_near, shadow_far);
            }
            else
            {
                light_projection = glm::perspective(glm::radians(90.0f), 1.0f, shadow_near, shadow_far);
            }

            glm::vec3 center = light.position;
            glm::mat4 light_view = glm::lookAt(
                center, (center + (glm::normalize(glm::vec3(light.direction)) * 5.0f)),
                glm::vec3(0.0, 1.0, 0.0));
            glm::mat4 light_space = light_projection * light_view;
            light.light_space_matrix = light_space;

            shadow_shader->bind();
            shadow_shader->set_mat4("lightSpaceMatrix", light_space);
            render_geometry();
            render_skinned_geometry();
        }
    }


    void Renderer::point_shadow_pass()
    {
        OpenGLDebugScope scope("Renderer::point_shadow_pass");
        auto point_shadow_shader = get_shader_by_name("point_shadow");

        for (auto &pair: _lights)
        {
            auto& rl = pair.second;
            if (rl.light.type != Point)
            {
                continue;
            }
            if (!pair.second.cast_shadows)
            {
                continue;
            }
            if (!pair.second.dirty)
            {
                continue;
            }

            rl.dirty = false;

            if (rl.light.shadow_handle == 0)
            {
                //get a new handle!
                if (_point_shadow_queue.empty())
                {
                    //empty list of ddir shadows, make new one
                    auto texture = Texture(create_point_shadow_texture(), 512, 512, 1);
                    texture.make_resident();
                    _point_shadow_maps[texture.get_bindless_handle()] = texture;
                    _point_shadow_queue.emplace(texture);
                }
                rl.light.shadow_handle = _point_shadow_queue.front().get_bindless_handle();
                _point_shadow_queue.pop();
            }

            glBindFramebuffer(GL_FRAMEBUFFER, point_shadow_fbo);
            glViewport(0, 0, _point_shadow_maps[rl.light.shadow_handle].get_width(),
                       _point_shadow_maps[rl.light.shadow_handle].get_height());
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                 _point_shadow_maps[rl.light.shadow_handle].get_handle(), 0);
            glClear(GL_DEPTH_BUFFER_BIT);
            glm::vec3 position = rl.light.position;

            point_shadow_shader->bind();
            point_shadow_shader->set_mat4("light_space_matrices", create_shadow_projection_matrices(position, rl.light.radius));
            for (int i = 0; i < 6; i++)
            {
                point_shadow_shader->set_int("idx", i);
                render_geometry();
                render_skinned_geometry();
            }
        }
    }

    void Renderer::shadow_pass()
    {
        OpenGLDebugScope scope("Renderer::shadow_pass");
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_CLAMP);
        dir_shadow_pass();
        point_shadow_pass();
        std::vector<Light> lights;
        for (auto &rl: _lights | std::views::values)
        {
            auto& light = lights.emplace_back(rl.light);
            light.outer_cutoff = glm::cos(glm::radians(light.outer_cutoff));
            light.inner_cutoff = glm::cos(glm::radians(light.inner_cutoff));
        }
        get_ssbo_by_name("lights")->update(sizeof(Light) * lights.size(), lights.data());
        glCullFace(GL_BACK);
        glDisable(GL_DEPTH_CLAMP);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
    }
}
