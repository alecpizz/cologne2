//
// Created by alecpizz on 5/3/2025.
//
#include <engine/Engine.h>

#include "DebugScope.h"
#include "Renderer.h"
#include "Shader.h"
#include "../Scene.h"

namespace cologne
{
    std::vector<float> shadowCascadeLevels;
    uint32_t shadow_fbo = 0;
    uint32_t shadow_cascade_ubo = 0;
    uint32_t shadow_size = 4096;
    uint32_t dir_shadow_size = 1024;
    float zMulti = 10.0f;
    float shadow_near = 0.1f;
    float shadow_far = 1200.0f;
    glm::vec3 _dir_shadow_offset = glm::vec3(0.0f);

    glm::mat4 get_light_space_matrix(const float near_plane, const float far_plane, glm::vec3 light_dir);

    std::vector<glm::mat4> get_light_space_matrices(glm::vec3 light_dir);

    std::vector<glm::vec4> get_frustum_corners_world_space(const glm::mat4 &proj, const glm::mat4 &view);

    std::vector<glm::vec4> get_frustum_corners_world_space(const glm::mat4 &proj_view);

    glm::mat4 get_light_space_matrix(const float near_plane, const float far_plane, glm::vec3 light_dir)
    {
        const auto proj = glm::perspective(
            Engine::get_camera()->get_fov(),
            (float) Engine::get_window()->get_width() / (float) Engine::get_window()->get_height(), near_plane,
            far_plane);
        const auto corners = get_frustum_corners_world_space(proj, Engine::get_camera()->get_view_matrix());

        glm::vec3 center = glm::vec3(0, 0, 0);
        for (const auto &v: corners)
        {
            center += glm::vec3(v);
        }
        center /= corners.size();
        const auto lightView = glm::lookAt(center - light_dir, center, glm::vec3(0.0f, 1.0f, 0.0f));

        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();
        for (const auto &v: corners)
        {
            const auto trf = lightView * v;
            minX = std::min(minX, trf.x);
            maxX = std::max(maxX, trf.x);
            minY = std::min(minY, trf.y);
            maxY = std::max(maxY, trf.y);
            minZ = std::min(minZ, trf.z);
            maxZ = std::max(maxZ, trf.z);
        }

        // Tune this parameter according to the scene
        if (minZ < 0)
        {
            minZ *= zMulti;
        } else
        {
            minZ /= zMulti;
        }
        if (maxZ < 0)
        {
            maxZ /= zMulti;
        } else
        {
            maxZ *= zMulti;
        }

        const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
        return lightProjection * lightView;
    }

    std::vector<glm::mat4> get_light_space_matrices(glm::vec3 light_dir)
    {
        std::vector<glm::mat4> ret;
        for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
        {
            if (i == 0)
            {
                ret.push_back(get_light_space_matrix(shadow_near, shadowCascadeLevels[i], light_dir));
            } else if (i < shadowCascadeLevels.size())
            {
                ret.push_back(get_light_space_matrix(shadowCascadeLevels[i - 1],
                    shadowCascadeLevels[i], light_dir));
            } else
            {
                ret.push_back(get_light_space_matrix(shadowCascadeLevels[i - 1], shadow_far, light_dir));
            }
        }
        return ret;
    }


    std::vector<glm::vec4> get_frustum_corners_world_space(const glm::mat4 &proj, const glm::mat4 &view)
    {
        return get_frustum_corners_world_space(proj * view);
    }

    std::vector<glm::vec4> get_frustum_corners_world_space(const glm::mat4 &proj_view)
    {
        const auto inv = glm::inverse(proj_view);

        std::vector<glm::vec4> frustumCorners;
        for (unsigned int x = 0; x < 2; ++x)
        {
            for (unsigned int y = 0; y < 2; ++y)
            {
                for (unsigned int z = 0; z < 2; ++z)
                {
                    const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                    frustumCorners.push_back(pt / pt.w);
                }
            }
        }

        return frustumCorners;
    }

    void Renderer::init_shadow()
    {
        shadowCascadeLevels.push_back(shadow_far / 50.0f);
        shadowCascadeLevels.push_back(shadow_far / 25.0f);
        shadowCascadeLevels.push_back(shadow_far / 10.0f);
        shadowCascadeLevels.push_back(shadow_far / 2.0f);
        Engine::get_debug_ui()->add_float_entry("ZMulti", zMulti);
        Engine::get_debug_ui()->add_float_entry("Shadow Far Plane", shadow_far);
        Engine::get_debug_ui()->add_float_entry("Shadow near Plane", shadow_near);


        glGenFramebuffers(1, &shadow_fbo);
        glGenTextures(1, &_shadow_depth);
        glBindTexture(GL_TEXTURE_2D_ARRAY, _shadow_depth);

        int32_t cascade_amount = static_cast<int32_t>(shadowCascadeLevels.size()) + 1;

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, shadow_size, shadow_size,
                     cascade_amount, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        constexpr float white[] = {1.0f, 1.0f, 1.0f, 1.0f};

        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, white);


        glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _shadow_depth, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERROR("Framebuffer is not complete!");
            return;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        glGenBuffers(1, &shadow_cascade_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, shadow_cascade_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, shadow_cascade_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        _dir_shadow_fbo.create("dir_shadow_fbo", dir_shadow_size, dir_shadow_size);
        _dir_shadow_fbo.create_depth_attachment(GL_DEPTH_COMPONENT16, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER);
        _dir_shadow_fbo.set_empty();
        Engine::get_debug_ui()->add_image_entry("dir_shadow", _dir_shadow_fbo.get_depth_attachment_handle(),
                                                glm::vec2(dir_shadow_size));
    }

    void Renderer::update_shadow(const Shader &shader)
    {
        shader.bind();
        for (size_t i = 0; i < shadowCascadeLevels.size(); i++)
        {
            shader.set_float(std::string("cascadePlaneDistances[" + std::to_string(i) + "]").c_str(),
                             shadowCascadeLevels[i]);
        }
        shader.set_float("far_plane", shadow_far);
        shader.set_int("cascadeCount", shadowCascadeLevels.size());
    }


    void Renderer::shadow_pass(Scene &scene)
    {
        DebugScope scope("Renderer::shadow_pass");
        auto light = get_directional_light();
        shadowCascadeLevels[0] = (shadow_far / 50.0f);
        shadowCascadeLevels[1] = (shadow_far / 25.0f);
        shadowCascadeLevels[2] = (shadow_far / 10.0f);
        shadowCascadeLevels[3] = (shadow_far / 2.0f);
        const auto light_matrices = get_light_space_matrices(light.direction);
        glBindBuffer(GL_UNIFORM_BUFFER, shadow_cascade_ubo);
        for (size_t i = 0; i < light_matrices.size(); i++)
        {
            glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &light_matrices[i]);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        auto shader = get_shader_by_name("shadowmap");
        shader->bind();
        shader->set_vec3("light.direction", glm::value_ptr(light.direction));
        shader->set_vec3("light.color", glm::value_ptr(light.color));
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
        glViewport(0, 0, shadow_size, shadow_size);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_CLAMP);

        for (size_t i = 0; i < scene.get_model_count(); i++)
        {
            auto model = scene.get_model_by_index(i);
            if (!model->get_active())
            {
                continue;
            }
            shader->set_mat4("model", glm::value_ptr(model->get_transform()->get_model_matrix()));
            for (size_t j = 0; j < model->get_num_meshes(); j++)
            {
                Mesh mesh = model->get_meshes()[j];
                Material mat = model->get_materials()[mesh.get_material_index()];
                mat.albedo.bind(ALBEDO_INDEX);
                mesh.draw();
            }
        }

        _dir_shadow_fbo.bind();
        _dir_shadow_fbo.set_viewport();
        glClear(GL_DEPTH_BUFFER_BIT);
        shader = get_shader_by_name("dir_shadow");
        shader->bind();
        glm::mat4 light_projection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, shadow_near, shadow_far);
        auto dir_light = get_directional_light();
        glm::vec3 center = dir_light.position;
        glm::mat4 light_view = glm::lookAt(center, (center + (dir_light.direction * 5.0f)),
            glm::vec3(0.0, 1.0, 0.0));
        draw_line(center, (center + (dir_light.direction * 5.0f)), glm::vec3(1.0, 0.0, 0.0));
        glm::mat4 light_space = light_projection * light_view;
        _dir_light_space = light_space;
        shader->set_mat4("lightSpaceMatrix", glm::value_ptr(_dir_light_space));

        for (size_t i = 0; i < scene.get_model_count(); i++)
        {
            auto model = scene.get_model_by_index(i);
            if (!model->get_active())
            {
                continue;
            }
            shader->set_mat4("model", glm::value_ptr(model->get_transform()->get_model_matrix()));
            for (size_t j = 0; j < model->get_num_meshes(); j++)
            {
                Mesh mesh = model->get_meshes()[j];
                Material mat = model->get_materials()[mesh.get_material_index()];
                mat.albedo.bind(ALBEDO_INDEX);
                mesh.draw();
            }
        }


        glCullFace(GL_BACK);
        glDisable(GL_DEPTH_CLAMP);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
    }
}
