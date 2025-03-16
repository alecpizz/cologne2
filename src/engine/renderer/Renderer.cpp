//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"

#include <engine/Engine.h>
#include <engine/Input.h>
#include "Material.h"
#include "Light.h"
#include "../Scene.h"
#include "Shader.h"
#include "HDRTexture.h"

namespace goon
{
#define ALBEDO_INDEX 0
#define AO_INDEX 1
#define METALLIC_INDEX 2
#define NORMAL_INDEX 3
#define ROUGHNESS_INDEX 4
#define EMISSION_INDEX 5
#define IRRADIANCE_INDEX 6
#define PREFILTER_INDEX 7
#define BRDF_INDEX 8
#define SKYBOX_INDEX 0

    struct Renderer::Impl
    {
        std::unique_ptr<Shader> lit_shader = nullptr;
        std::unique_ptr<Shader> skybox_shader = nullptr;
        std::vector<Light> lights;
        Texture env_cubemap;
        Texture env_irradiance;
        Texture env_prefilter;
        Texture env_brdf;

        void init()
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glEnable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
            glCullFace(GL_BACK);
        }

        void init_lit_shader()
        {
            lit_shader = std::make_unique<Shader>(RESOURCES_PATH "shaders/lit.vert",
                                                  RESOURCES_PATH "shaders/lit.frag");
            lit_shader->bind();
            lit_shader->set_int("texture_albedo", ALBEDO_INDEX);
            lit_shader->set_int("texture_normal", NORMAL_INDEX);
            lit_shader->set_int("texture_roughness", ROUGHNESS_INDEX);
            lit_shader->set_int("texture_metallic", METALLIC_INDEX);
            lit_shader->set_int("texture_ao", AO_INDEX);
            lit_shader->set_int("texture_emission", EMISSION_INDEX);
            lit_shader->set_int("irradiance_map", IRRADIANCE_INDEX);
            lit_shader->set_int("prefilter_map", PREFILTER_INDEX);
            lit_shader->set_int("brdf", BRDF_INDEX);

            skybox_shader = std::make_unique<Shader>(RESOURCES_PATH "shaders/skybox.vert",
                                                     RESOURCES_PATH "shaders/skybox.frag");
            skybox_shader->bind();
            skybox_shader->set_int("environment_map", SKYBOX_INDEX);
        }

        void gen_environment_map(const std::string &path)
        {
            auto hdr = HDRTexture(RESOURCES_PATH "newport_loft.hdr");

            Shader environment(RESOURCES_PATH "shaders/environment.vert", RESOURCES_PATH "shaders/environment.frag");

            uint32_t captureFBO, captureRBO;
            glGenFramebuffers(1, &captureFBO);
            glGenRenderbuffers(1, &captureRBO);

            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

            uint32_t envCubemap;
            glGenTextures(1, &envCubemap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
            for (uint32_t i = 0; i < 6; i++)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 captureViews[] =
            {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
            };

            environment.bind();
            environment.set_int("equirectangular_map", 0);
            environment.set_mat4("projection", glm::value_ptr(captureProjection));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, hdr.get_handle());

            glViewport(0, 0, 512, 512);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            for (uint32_t i = 0; i < 6; i++)
            {
                environment.set_mat4("view", glm::value_ptr(captureViews[i]));
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                render_cube();
            }
            // glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
            // glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            env_cubemap = Texture(envCubemap, 512, 512, 3);
        }

        void gen_irradiance_map()
        {
            uint32_t irradiance_handle = 0;
            glGenTextures(1, &irradiance_handle);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_handle);
            for (uint32_t i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                             GL_RGB16F, 32, 32, 0,
                             GL_RGB, GL_FLOAT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            uint32_t captureFBO = 0;
            uint32_t captureRBO = 0;
            glCreateFramebuffers(1, &captureFBO);
            glCreateRenderbuffers(1, &captureRBO);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER, captureRBO);


            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 captureViews[] =
            {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
            };

            Shader irradiance_shader(RESOURCES_PATH "shaders/environment.vert",
                                     RESOURCES_PATH "shaders/irradiance.frag");
            irradiance_shader.bind();
            irradiance_shader.set_int("environment_map", 0);
            irradiance_shader.set_mat4("projection", glm::value_ptr(captureProjection));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap.get_handle());
            glViewport(0, 0, 32, 32);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            for (uint32_t i = 0; i < 6; ++i)
            {
                irradiance_shader.set_mat4("view", glm::value_ptr(captureViews[i]));
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_handle, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                render_cube();
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            env_irradiance = Texture(irradiance_handle, 32, 32, 3);
            glDeleteFramebuffers(1, &captureFBO);
            glDeleteRenderbuffers(1, &captureRBO);
        }

        void gen_prefilter_map()
        {
            uint32_t prefilterMap;
            glGenTextures(1, &prefilterMap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
            for (uint32_t i = 0; i < 6; i++)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 captureViews[] =
            {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
            };

            Shader prefilter(RESOURCES_PATH "shaders/environment.vert", RESOURCES_PATH "shaders/prefilter.frag");
            prefilter.bind();
            prefilter.set_int("environment_map", 0);
            prefilter.set_mat4("projection", glm::value_ptr(captureProjection));


            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap.get_handle());
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            GLuint captureFBO, captureRBO;
            glGenFramebuffers(1, &captureFBO);
            glGenRenderbuffers(1, &captureRBO);

            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            uint32_t maxMipLevels = 5;
            for (uint32_t mip = 0; mip < maxMipLevels; mip++)
            {
                uint32_t mipWidth = 128 * std::pow(0.5, mip);
                uint32_t mipHeight = 128 * std::pow(0.5, mip);

                glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
                glViewport(0, 0, mipWidth, mipHeight);

                float roughness = (float) mip / (float) (maxMipLevels - 1);
                prefilter.set_float("roughness", roughness);

                for (uint32_t i = 0; i < 6; i++)
                {
                    prefilter.set_mat4("view", glm::value_ptr(captureViews[i]));
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    render_cube();
                }
            }
            env_prefilter = Texture(prefilterMap, 128, 128, 3);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteRenderbuffers(1, &captureRBO);
            glDeleteFramebuffers(1, &captureFBO);
        }

        void gen_brdf_map()
        {
            uint32_t brdf_handle;
            glGenTextures(1, &brdf_handle);
            glBindTexture(GL_TEXTURE_2D, brdf_handle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F,
                         512, 512, 0, GL_RG, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            uint32_t captureFBO, captureRBO;
            glGenFramebuffers(1, &captureFBO);
            glGenRenderbuffers(1, &captureRBO);

            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_handle, 0);
            glViewport(0, 0, 512, 512);

            Shader brdf_shader(RESOURCES_PATH "shaders/brdf.vert", RESOURCES_PATH "shaders/brdf.frag");
            brdf_shader.bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            render_quad();
            env_brdf = Texture(brdf_handle, 512, 512, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &captureFBO);
            glDeleteRenderbuffers(1, &captureRBO);
            glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
        }

        void add_light(Light light)
        {
            lights.emplace_back(light);
            size_t light_idx = lights.size() - 1;
            lit_shader->bind();
            lit_shader->set_vec3(std::string("lights[" + std::to_string(light_idx) + "].position").c_str(),
                                 glm::value_ptr(lights[light_idx].position));
            lit_shader->set_vec3(std::string("lights[" + std::to_string(light_idx) + "].color").c_str(),
                                 glm::value_ptr(lights[light_idx].color));
            lit_shader->set_float(std::string("lights[" + std::to_string(light_idx) + "].radius").c_str(),
                                  lights[light_idx].radius);
            lit_shader->set_int(std::string("lights[" + std::to_string(light_idx) + "].type").c_str(),
                                lights[light_idx].type);
            lit_shader->set_float(std::string("lights[" + std::to_string(light_idx) + "].strength").c_str(),
                                  lights[light_idx].strength);
            lit_shader->set_vec3(std::string("lights[" + std::to_string(light_idx) + "].direction").c_str(),
                                 glm::value_ptr(lights[light_idx].direction));
            lit_shader->set_int("num_lights", lights.size());
        }

        void render_cube()
        {
            static GLuint cubeVAO = 0;
            static GLuint cubeVBO = 0;

            if (cubeVAO == 0)
            {
                float vertices[] = {
                    // back face
                    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                    1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                    1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                    1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                    -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
                    // front face
                    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                    1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
                    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                    -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
                    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                    // left face
                    -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                    -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
                    -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                    -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                    -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                    -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                    // right face
                    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                    1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
                    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                    1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
                    // bottom face
                    -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                    1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
                    1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                    1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                    -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                    -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                    // top face
                    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                    1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                    1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
                    1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                    -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
                };

                glGenVertexArrays(1, &cubeVAO);
                glGenBuffers(1, &cubeVBO);
                // fill buffer
                glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                // link vertex attributes
                glBindVertexArray(cubeVAO);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }

        void render_quad()
        {
            static uint32_t quad_vao = 0;
            static uint32_t quad_vbo = 0;
            if (quad_vao == 0)
            {
                float quadVertices[] = {
                    // positions        // texture Coords
                    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                };
                // setup plane VAO
                glGenVertexArrays(1, &quad_vao);
                glGenBuffers(1, &quad_vbo);
                glBindVertexArray(quad_vao);
                glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
            }
            glBindVertexArray(quad_vao);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        }

        void bind_lights(const Shader &shader)
        {
            shader.bind();
            for (size_t i = 0; i < lights.size(); i++)
            {
                lit_shader->set_vec3(std::string("lights[" + std::to_string(i) + "].position").c_str(),
                                     glm::value_ptr(lights[i].position));
                lit_shader->set_vec3(std::string("lights[" + std::to_string(i) + "].color").c_str(),
                                     glm::value_ptr(lights[i].color));
                lit_shader->set_float(std::string("lights[" + std::to_string(i) + "].radius").c_str(),
                                      lights[i].radius);
                lit_shader->set_int(std::string("lights[" + std::to_string(i) + "].type").c_str(),
                                    lights[i].type);
                lit_shader->set_float(std::string("lights[" + std::to_string(i) + "].strength").c_str(),
                                      lights[i].strength);
                lit_shader->set_vec3(std::string("lights[" + std::to_string(i) + "].direction").c_str(),
                                     glm::value_ptr(lights[i].direction));
            }
            lit_shader->set_int("num_lights", lights.size());
        }


        void lit_pass(Scene &scene)
        {
            lit_shader->bind();
            lit_shader->set_vec3("camera_pos", glm::value_ptr(Engine::get_camera()->get_position()));
            lit_shader->set_mat4("projection", &Engine::get_camera()->get_projection_matrix()[0][0]);
            lit_shader->set_mat4("view", &Engine::get_camera()->get_view_matrix()[0][0]);
            env_irradiance.bind(IRRADIANCE_INDEX);
            env_prefilter.bind(PREFILTER_INDEX);
            env_brdf.bind(BRDF_INDEX);
            for (size_t i = 0; i < scene.get_model_count(); i++)
            {
                auto model = scene.get_model_by_index(i);
                lit_shader->set_mat4("model", glm::value_ptr(model->get_transform()->get_model_matrix()));
                for (size_t j = 0; j < model->get_num_meshes(); j++)
                {
                    Mesh mesh = model->get_meshes()[j];
                    Material mat = model->get_materials()[mesh.get_material_index()];
                    mat.albedo.bind(ALBEDO_INDEX);
                    mat.ao.bind(AO_INDEX);
                    mat.metallic.bind(METALLIC_INDEX);
                    mat.roughness.bind(ROUGHNESS_INDEX);
                    mat.normal.bind(NORMAL_INDEX);
                    mat.emission.bind(EMISSION_INDEX);
                    mesh.draw();
                    glBindTextureUnit(1, 0);
                    glBindTextureUnit(2, 0);
                    glBindTextureUnit(3, 0);
                    glBindTextureUnit(4, 0);
                    glBindTextureUnit(5, 0);
                }
            }
        }

        void skybox_pass()
        {
            glDepthFunc(GL_LEQUAL);
            skybox_shader->bind();
            skybox_shader->set_mat4("projection", glm::value_ptr(Engine::get_camera()->get_projection_matrix()));
            skybox_shader->set_mat4("view", glm::value_ptr(Engine::get_camera()->get_view_matrix()));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap.get_handle());
            render_cube();
            glDepthFunc(GL_LESS);
        }
    };

    Renderer::~Renderer()
    {
        delete _impl;
    }


    void Renderer::render_scene(Scene &scene)
    {
        //indirect pass
        //shadow maps
        //geometry pass
        //culling pass
        //lighting pass
        //skybox pass
        //post processing pass
        //any debug visuals pass

        if (goon::Input::key_pressed(Input::Key::H))
        {
            //hot reload shaders
            reload_shaders();
        }

        _impl->lit_pass(scene);
        _impl->skybox_pass();
    }

    void Renderer::reload_shaders()
    {
        _impl->lit_shader.release();
        _impl->init_lit_shader();
        _impl->bind_lights(*_impl->lit_shader);
        LOG_INFO("RELOADED SHADERS");
    }

    Light &Renderer::get_directional_light() const
    {
        for (auto &light: _impl->lights)
        {
            if (light.type == LightType::Directional)
            {
                return light;
            }
        }
        LOG_WARN("No light found");
        return _impl->lights[0];
    }

    void Renderer::set_directional_light(glm::vec3 direction)
    {
        auto &directional_light = get_directional_light();
        directional_light.direction = direction;
        reload_shaders();
    }

    Renderer::Renderer()
    {
        _impl = new Impl();
        _impl->init();
        _impl->init_lit_shader();
        _impl->gen_environment_map("newport_loft.hdr");
        _impl->gen_irradiance_map();
        _impl->gen_prefilter_map();
        _impl->gen_brdf_map();
        _impl->add_light(Light(glm::vec3(-10.0f, 10.0f, 10.0f), glm::vec3(-2.0f, -1.0f, -0.3f),
                               glm::vec3(300.0f, 300.0f, 300.0f), 6.0f, 1.0f,
                               LightType::Directional));
        _impl->add_light(Light(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f), 6.0f, 1.0f,
                               LightType::Point));
        _impl->add_light(Light(glm::vec3(-10.0f, -10.0f, 10.0f), glm::vec3(0.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f), 6.0f, 1.0f,
                               LightType::Point));
        _impl->add_light(Light(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(0.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f), 6.0f, 1.0f,
                               LightType::Point));
    }
}
