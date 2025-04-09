//
// Created by alecp on 4/8/2025.
//

#include "Probe.h"

#include <engine/Engine.h>

#include "Shader.h"

namespace goon
{
    Probe::Probe(glm::vec3 position)
    {
        _position = position;
        const int32_t size = 64;

        //albedo
        glGenTextures(1, &_albedo_handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _albedo_handle);
        for (uint32_t i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         GL_RGBA16F, size, size, 0,
                         GL_RGBA, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //position
        glGenTextures(1, &_position_handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _position_handle);
        for (uint32_t i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         GL_RGB16F, size, size, 0,
                         GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //normal
        glGenTextures(1, &_normal_handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _normal_handle);
        for (uint32_t i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         GL_RGB16F, size, size, 0,
                         GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //orm
        glGenTextures(1, &_orm_handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _orm_handle);
        for (uint32_t i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         GL_RGB16F, size, size, 0,
                         GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //depth
        glGenTextures(1, &_depth_handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _depth_handle);
        for (uint32_t i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         GL_DEPTH_COMPONENT, size, size, 0,
                         GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }


    glm::vec3 Probe::get_position()
    {
        return _position;
    }

    void Probe::bake_geo(Shader& shader)
    {
        //framebuffer
        uint32_t fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                               _albedo_handle, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                               _position_handle, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                               _normal_handle, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                               _orm_handle, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                               _depth_handle, 0);

        uint32_t draw_buffers[4] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
        };
        glDrawBuffers(4, draw_buffers);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERROR("Framebuffer is not complete");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return;
        }

        glm::vec3 capture_pos = _position;
        glm::mat4 capture_proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 capture_views[] = {
            glm::lookAt(capture_pos, capture_pos + glm::vec3(1.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(capture_pos, capture_pos + glm::vec3(-1.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(capture_pos, capture_pos + glm::vec3(0.0f, 1.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(capture_pos, capture_pos + glm::vec3(0.0f, -1.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(capture_pos, capture_pos + glm::vec3(0.0f, 0.0f, 1.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(capture_pos, capture_pos + glm::vec3(0.0f, 0.0f, -1.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f))
        };

        shader.bind();
        shader.set_mat4("projection", glm::value_ptr(capture_proj));
        for (uint32_t face = 0; face < 6; face++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   _albedo_handle, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   _position_handle, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   _normal_handle, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   _orm_handle, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   _depth_handle, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            auto scene = Engine::get_scene();
            shader.set_mat4("view", glm::value_ptr(capture_views[face]));
            for (size_t i = 0; i < scene->get_model_count(); i++)
            {
                auto model = scene->get_model_by_index(i);
                if (!model->get_active())
                {
                    continue;
                }
                shader.set_mat4("model", glm::value_ptr(model->get_transform()->get_model_matrix()));
                for (size_t j = 0; j < model->get_num_meshes(); j++)
                {
                    auto mesh = model->get_meshes()[j];
                    auto mat = model->get_materials()[mesh.get_material_index()];
                    mat.bind_all();
                    mesh.draw();
                }
            }
        }

        glDeleteFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        //make em bindless
        _albedo_bindless = glGetTextureHandleARB(_albedo_handle);
        glMakeTextureHandleResidentARB(_albedo_bindless);
        _normal_bindless = glGetTextureHandleARB(_normal_handle);
        glMakeTextureHandleResidentARB(_normal_bindless);
        _position_bindless = glGetTextureHandleARB(_position_handle);
        glMakeTextureHandleResidentARB(_position_bindless);
        _orm_bindless = glGetTextureHandleARB(_orm_handle);
        glMakeTextureHandleResidentARB(_orm_bindless);
        _depth_bindless = glGetTextureHandleARB(_depth_handle);
        glMakeTextureHandleResidentARB(_depth_bindless);
    }

    void Probe::light()
    {
    }

    void Probe::cleanup()
    {
        glMakeTextureHandleNonResidentARB(_albedo_bindless);
        glDeleteTextures(1, &_albedo_handle);
        glMakeTextureHandleNonResidentARB(_normal_bindless);
        glDeleteTextures(1, &_normal_handle);
        glMakeTextureHandleNonResidentARB(_orm_bindless);
        glDeleteTextures(1, &_orm_handle);
        glMakeTextureHandleNonResidentARB(_depth_bindless);
        glDeleteTextures(1, &_depth_handle);
        glMakeTextureHandleNonResidentARB(_position_bindless);
        glDeleteTextures(1, &_position_handle);
    }

    uint32_t Probe::get_albedo_handle() const
    {
        return _albedo_handle;
    }

    uint32_t Probe::get_normal_handle() const
    {
        return _normal_handle;
    }

    uint32_t Probe::get_position_handle() const
    {
        return _position_handle;
    }

    uint32_t Probe::get_depth_handle() const
    {
        return _depth_handle;
    }

    uint32_t Probe::get_orm_handle() const
    {
        return _orm_handle;
    }

    uint64_t Probe::get_albedo_bindless() const
    {
        return _albedo_bindless;
    }

    uint64_t Probe::get_normal_bindless() const
    {
        return _normal_bindless;
    }

    uint64_t Probe::get_position_bindless() const
    {
        return _position_bindless;
    }

    uint64_t Probe::get_depth_bindless() const
    {
        return _depth_bindless;
    }

    uint64_t Probe::get_orm_bindless() const
    {
        return _orm_bindless;
    }
}
