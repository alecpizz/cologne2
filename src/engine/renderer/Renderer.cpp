//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"

#include <engine/Engine.h>
#include <engine/Input.h>

#include "DebugRenderer.h"
#include "FrameBuffer.h"
#include "Material.h"
#include "Light.h"
#include "../Scene.h"
#include "Shader.h"
#include "HDRTexture.h"
#include "TextRenderer.h"
#include "Probe.h"

namespace cologne
{
#define IRRADIANCE_INDEX 6
#define PREFILTER_INDEX 7
#define BRDF_INDEX 8
#define SHADOW_INDEX 9


    struct Renderer::Impl
    {
        std::shared_ptr<Shader> lit_shader = nullptr;
        std::shared_ptr<Shader> g_buffer_shader = nullptr;
        std::shared_ptr<Shader> skybox_shader = nullptr;
        std::shared_ptr<Shader> shadowmap_shader = nullptr;
        std::shared_ptr<Shader> fbo_debug_shader = nullptr;
        std::shared_ptr<Shader> probe_debug_shader = nullptr;
        std::shared_ptr<Shader> probe_lit_shader = nullptr;
        std::shared_ptr<DebugRenderer> debug_renderer = nullptr;
        std::shared_ptr<TextRenderer> text_renderer = nullptr;
        std::shared_ptr<Shader> probe_g_buffer_shader = nullptr;
        std::unordered_map<std::string, std::shared_ptr<Shader> > shaders = std::unordered_map<std::string, std::shared_ptr<Shader> >();
        std::vector<Light> lights;
        std::vector<float> shadowCascadeLevels;
        Texture env_cubemap;
        Texture env_irradiance;
        Texture env_prefilter;
        Texture env_brdf;
        Texture shadow_map;
        uint32_t shadow_fbo = 0;
        uint32_t g_buffer = 0;
        uint32_t g_position = 0;
        uint32_t g_albedo;
        uint32_t g_normal;
        uint32_t g_metallic_roughness_ao;
        uint32_t g_depth;
        uint32_t rsm_fbo = 0;
        uint32_t rsm_depth = 0;
        uint32_t shadow_cascade_ubo = 0;
        uint32_t rsm_size = 4096;
        float zMulti = 10.0f;
        float shadow_near = 0.1f;
        float shadow_far = 1200.0f;


        //PROBES
        int32_t probe_depth = 8;
        int32_t probe_height = 8;
        int32_t probe_width = 12;
        float probe_spacing = 1.0f;
        glm::vec3 probe_volume_origin = glm::vec3(-6.0f, 0.1f, -4.0f);
        std::vector<Probe> probes;
        uint32_t probe_lighting = 0;
        uint32_t probe_gbuffer_ssbo = 0;
        uint32_t probe_positions = 0;

        void init()
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
            glEnable(GL_MULTISAMPLE);
            debug_renderer = std::make_unique<DebugRenderer>();
            text_renderer = std::unique_ptr<TextRenderer>(
                new TextRenderer(RESOURCES_PATH "fonts/Montserrat-Regular.ttf"));
            shadowCascadeLevels.push_back(shadow_far / 50.0f);
            shadowCascadeLevels.push_back(shadow_far / 25.0f);
            shadowCascadeLevels.push_back(shadow_far / 10.0f);
            shadowCascadeLevels.push_back(shadow_far / 2.0f);
            Engine::get_debug_ui()->add_float_entry("ZMulti", zMulti);
            Engine::get_debug_ui()->add_float_entry("Shadow Far Plane", shadow_far);
            Engine::get_debug_ui()->add_float_entry("Shadow near Plane", shadow_near);
            Engine::get_debug_ui()->add_int_entry("Probe Depth", probe_depth);
            Engine::get_debug_ui()->add_int_entry("Probe Width", probe_width);
            Engine::get_debug_ui()->add_int_entry("Probe Height", probe_height);
            Engine::get_debug_ui()->add_float_entry("Probe Spacing", probe_spacing);
            Engine::get_debug_ui()->add_vec3_entry("Probe Origin", probe_volume_origin);
        }

        void init_shaders()
        {
            lit_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/lit.vert",
                                                  RESOURCES_PATH "shaders/lit.frag");
            lit_shader->bind();
            for (size_t i = 0; i < shadowCascadeLevels.size(); i++)
            {
                lit_shader->set_float(std::string("cascadePlaneDistances[" + std::to_string(i) + "]").c_str(),
                                      shadowCascadeLevels[i]);
            }
            lit_shader->set_float("farPlane", shadow_far);
            lit_shader->set_int("cascadeCount", shadowCascadeLevels.size());

            g_buffer_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/gbuffer.vert",
                                                       RESOURCES_PATH "shaders/gbuffer.frag");

            skybox_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/skybox.vert",
                                                     RESOURCES_PATH "shaders/skybox.frag");
            fbo_debug_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/framebufferoutput.vert",
                                                        RESOURCES_PATH "shaders/framebufferoutput.frag");
            shadowmap_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/shadowmap2.vert",
                                                        RESOURCES_PATH "shaders/shadowmap2.frag",
                                                        RESOURCES_PATH "shaders/shadowmap2.geom");
            probe_debug_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/probe_debug.vert",
                                                          RESOURCES_PATH "shaders/probe_debug.frag");
            probe_lit_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/probe_lit.comp");
            probe_g_buffer_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/probe_g_buffer.vert",
                                  RESOURCES_PATH "shaders/probe_g_buffer.frag");
            shaders.clear();
            shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("lit", lit_shader));
            shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("gbuffer", g_buffer_shader));
            shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("skybox", skybox_shader));
            shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("shadowmap", shadowmap_shader));
            shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("probe_debug", probe_debug_shader));
            shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("probe_lit", probe_lit_shader));
            shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("probe_g_buffer", probe_g_buffer_shader));
        }

        void init_gbuffer(uint32_t width, uint32_t height)
        {
            if (g_buffer != 0)
            {
                glDeleteFramebuffers(1, &g_buffer);
            }
            glGenFramebuffers(1, &g_buffer);
            glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

            //position
            glGenTextures(1, &g_position);
            glBindTexture(GL_TEXTURE_2D, g_position);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
                         GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   g_position, 0);

            //normal
            glGenTextures(1, &g_normal);
            glBindTexture(GL_TEXTURE_2D, g_normal);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
                         GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                                   g_normal, 0);

            //albedo
            glGenTextures(1, &g_albedo);
            glBindTexture(GL_TEXTURE_2D, g_albedo);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                                   g_albedo, 0);

            //orm
            glGenTextures(1, &g_metallic_roughness_ao);
            glBindTexture(GL_TEXTURE_2D, g_metallic_roughness_ao);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
                                   g_metallic_roughness_ao, 0);

            //depth

            glGenTextures(1, &g_depth);
            glBindTexture(GL_TEXTURE_2D, g_depth);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                         width, height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, g_depth, 0);

            uint32_t attachments[4] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3
            };
            glDrawBuffers(4, attachments);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                LOG_ERROR("Framebuffer is not complete! %s", glGetError());
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void light_probes()
        {
            //light all the probes, take in 256 random points and do direct lighting on each point. generate SH coefficients from resulting lighting
            probe_lit_shader->bind();
            probe_lit_shader->set_int("probe_depth", probe_depth);
            probe_lit_shader->set_int("probe_width", probe_width);
            probe_lit_shader->set_int("probe_height", probe_height);
            probe_lit_shader->set_float("probe_spacing", probe_spacing);
            probe_lit_shader->set_vec3("origin", glm::value_ptr(probe_volume_origin));
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, probe_gbuffer_ssbo);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, probe_positions);

            for (size_t i = 0; i < lights.size(); ++i)
            {
                probe_lit_shader->set_vec3(std::string("lights[" + std::to_string(i) + "].position").c_str(),
                                           glm::value_ptr(lights[i].position));
                probe_lit_shader->set_vec3(std::string("lights[" + std::to_string(i) + "].direction").c_str(),
                                           glm::value_ptr(lights[i].direction));
                probe_lit_shader->set_vec3(std::string("lights[" + std::to_string(i) + "].color").c_str(),
                                           glm::value_ptr(lights[i].color));
                probe_lit_shader->set_float(std::string("lights[" + std::to_string(i) + "].radius").c_str(),
                                            (lights[i].radius));
                probe_lit_shader->set_float(std::string("lights[" + std::to_string(i) + "].strength").c_str(),
                                            (lights[i].strength));
                probe_lit_shader->set_int(std::string("lights[" + std::to_string(i) + "].type").c_str(),
                                          lights[i].type);
            }
            probe_lit_shader->set_int("num_lights", lights.size());

            probe_lit_shader->dispatch(probes.size(), 1, 1);
            probe_lit_shader->wait();
            //store all SH coefficients in a buffer -> 3d texture
            //sample from the 8 nearest probes
        }

        void create_probe_g_buffers()
        {
            for (int32_t x = 0; x < probe_width; x++)
            {
                for (int32_t y = 0; y < probe_height; y++)
                {
                    for (int32_t z = 0; z < probe_depth; z++)
                    {
                        float x_pos = static_cast<float>(x) * probe_spacing;
                        float y_pos = static_cast<float>(y) * probe_spacing;
                        float z_pos = static_cast<float>(z) * probe_spacing;
                        glm::vec3 capture_pos = glm::vec3(x_pos, y_pos, z_pos);
                        capture_pos += probe_volume_origin;
                        auto &probe = probes.emplace_back(capture_pos);
                        probe.bake_geo(*probe_g_buffer_shader);
                    }
                }
            }
            LOG_INFO("Baked %d G B buffers", probes.size());
            //put probe info into a ssbo
            glCreateBuffers(1, &probe_gbuffer_ssbo);
            glCreateBuffers(1, &probe_positions);
            std::vector<uint64_t> gbuffer_handles;
            std::vector<glm::vec4> positions;
            for (auto &probe: probes)
            {
                gbuffer_handles.push_back(probe.get_albedo_bindless());
                gbuffer_handles.push_back(probe.get_normal_bindless());
                gbuffer_handles.push_back(probe.get_position_bindless());
                gbuffer_handles.push_back(probe.get_orm_bindless());
                positions.push_back(glm::vec4(probe.get_position(), 1.0));
            }
            glNamedBufferStorage(probe_gbuffer_ssbo, sizeof(uint64_t) * gbuffer_handles.size(),
                                 reinterpret_cast<const void *>(gbuffer_handles.data()), GL_DYNAMIC_STORAGE_BIT);
            glNamedBufferStorage(probe_positions, sizeof(glm::vec4) * positions.size(),
                reinterpret_cast<const void *>(positions.data()), GL_DYNAMIC_STORAGE_BIT);
        }

        void init_shadow_map()
        {
            glGenFramebuffers(1, &rsm_fbo);
            glGenTextures(1, &rsm_depth);
            glBindTexture(GL_TEXTURE_2D_ARRAY, rsm_depth);

            int32_t cascade_amount = static_cast<int32_t>(shadowCascadeLevels.size()) + 1;

            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, rsm_size, rsm_size,
                         cascade_amount, 0,
                         GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            constexpr float white[] = {1.0f, 1.0f, 1.0f, 1.0f};

            glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, white);


            glBindFramebuffer(GL_FRAMEBUFFER, rsm_fbo);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, rsm_depth, 0);
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
        }

        void gen_environment_map(const std::string &path)
        {
            auto hdr = HDRTexture(std::string(RESOURCES_PATH + path).c_str());

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
            environment.set_mat4("projection", &captureProjection[0][0]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, hdr.get_handle());

            glViewport(0, 0, 512, 512);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            for (uint32_t i = 0; i < 6; i++)
            {
                environment.set_mat4("view", &captureViews[i][0][0]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                render_cube();
            }
            glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            env_cubemap = Texture(envCubemap, 512, 512, 3);
            glDeleteFramebuffers(1, &captureFBO);
            glDeleteRenderbuffers(1, &captureRBO);
        }

        void gen_irradiance_map()
        {
            uint32_t captureFBO, captureRBO;
            glGenFramebuffers(1, &captureFBO);
            glGenRenderbuffers(1, &captureRBO);

            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

            uint32_t irradianceMap;
            glGenTextures(1, &irradianceMap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
            for (uint32_t i = 0; i < 6; i++)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
                             GL_RGB, GL_FLOAT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

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

            Shader irradianceShader(RESOURCES_PATH "shaders/environment.vert",
                                    RESOURCES_PATH "shaders/irradiance.frag");
            irradianceShader.bind();
            irradianceShader.set_mat4("projection", &captureProjection[0][0]);
            env_cubemap.bind(0);
            glViewport(0, 0, 32, 32);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

            for (uint32_t i = 0; i < 6; i++)
            {
                irradianceShader.set_mat4("view", &captureViews[i][0][0]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                render_cube();
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            env_irradiance = Texture(irradianceMap, 512, 512, 3);
        }

        void gen_prefilter_map()
        {
            uint32_t captureFBO, captureRBO;
            glGenFramebuffers(1, &captureFBO);
            glGenRenderbuffers(1, &captureRBO);

            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

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

            Shader prefilter(RESOURCES_PATH "shaders/environment.vert", RESOURCES_PATH "shaders/prefilter.frag");
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
            prefilter.bind();
            prefilter.set_int("environmentMap", 0);
            prefilter.set_mat4("projection", &captureProjection[0][0]);
            env_cubemap.bind(0);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

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
                    prefilter.set_mat4("view", &captureViews[i][0][0]);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    render_cube();
                }
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            env_prefilter = Texture(prefilterMap, 128, 128, 3);
            glDeleteFramebuffers(1, &captureFBO);
            glDeleteRenderbuffers(1, &captureRBO);
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
            static uint32_t cubeVAO = 0;
            static uint32_t cubeVBO = 0;

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

                glCreateBuffers(1, &cubeVBO);
                glNamedBufferStorage(cubeVBO, sizeof(vertices), vertices, GL_MAP_READ_BIT);
                glCreateVertexArrays(1, &cubeVAO);
                glVertexArrayVertexBuffer(cubeVAO, 0, cubeVBO, 0, 8 * sizeof(float));

                glEnableVertexArrayAttrib(cubeVAO, 0);
                glEnableVertexArrayAttrib(cubeVAO, 1);
                glEnableVertexArrayAttrib(cubeVAO, 2);

                glVertexArrayAttribFormat(cubeVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
                glVertexArrayAttribFormat(cubeVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
                glVertexArrayAttribFormat(cubeVAO, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));

                glVertexArrayAttribBinding(cubeVAO, 0, 0);
                glVertexArrayAttribBinding(cubeVAO, 1, 0);
                glVertexArrayAttribBinding(cubeVAO, 2, 0);
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

        void gbuffer_pass(Scene &scene)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
            glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            g_buffer_shader->bind();
            g_buffer_shader->set_mat4("projection", &Engine::get_camera()->get_projection_matrix()[0][0]);
            g_buffer_shader->set_mat4("view", &Engine::get_camera()->get_view_matrix()[0][0]);

            for (size_t i = 0; i < scene.get_model_count(); i++)
            {
                auto model = scene.get_model_by_index(i);
                if (!model->get_active())
                {
                    continue;
                }
                g_buffer_shader->set_mat4("model", glm::value_ptr(model->get_transform()->get_model_matrix()));
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

                    glBindTextureUnit(ALBEDO_INDEX, 0);
                    glBindTextureUnit(AO_INDEX, 0);
                    glBindTextureUnit(METALLIC_INDEX, 0);
                    glBindTextureUnit(ROUGHNESS_INDEX, 0);
                    glBindTextureUnit(NORMAL_INDEX, 0);
                    glBindTextureUnit(EMISSION_INDEX, 0);
                }
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void lit_pass(Scene &scene)
        {
            glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            lit_shader->bind();
            lit_shader->set_vec3("camera_pos", glm::value_ptr(Engine::get_camera()->get_position()));
            env_irradiance.bind(IRRADIANCE_INDEX);
            env_prefilter.bind(PREFILTER_INDEX);
            env_brdf.bind(BRDF_INDEX);
            lit_shader->set_float("far_plane", shadow_far);
            glBindTextureUnit(0, g_position);
            glBindTextureUnit(1, g_normal);
            glBindTextureUnit(2, g_albedo);
            glBindTextureUnit(3, g_metallic_roughness_ao);
            glBindTextureUnit(13, rsm_depth);
            render_quad();
            glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
            glBlitFramebuffer(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height(), 0, 0,
                              Engine::get_window()->get_width(), Engine::get_window()->get_height(),
                              GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void shadow_pass(Scene &scene)
        {
            shadowCascadeLevels[0] = (shadow_far / 50.0f);
            shadowCascadeLevels[1] = (shadow_far / 25.0f);
            shadowCascadeLevels[2] = (shadow_far / 10.0f);
            shadowCascadeLevels[3] = (shadow_far / 2.0f);
            const auto light_matrices = get_light_space_matrices();
            glBindBuffer(GL_UNIFORM_BUFFER, shadow_cascade_ubo);
            for (size_t i = 0; i < light_matrices.size(); i++)
            {
                glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &light_matrices[i]);
            }
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            shadowmap_shader->bind();
            shadowmap_shader->set_vec3("light.direction", glm::value_ptr(lights[0].direction));
            shadowmap_shader->set_vec3("light.color", glm::value_ptr(lights[0].color));
            glBindFramebuffer(GL_FRAMEBUFFER, rsm_fbo);
            glViewport(0, 0, rsm_size, rsm_size);
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
                shadowmap_shader->set_mat4("model", glm::value_ptr(model->get_transform()->get_model_matrix()));
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

        std::vector<glm::mat4> get_light_space_matrices()
        {
            std::vector<glm::mat4> ret;
            for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
            {
                if (i == 0)
                {
                    ret.push_back(get_light_space_matrix(shadow_near, shadowCascadeLevels[i]));
                } else if (i < shadowCascadeLevels.size())
                {
                    ret.push_back(get_light_space_matrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
                } else
                {
                    ret.push_back(get_light_space_matrix(shadowCascadeLevels[i - 1], shadow_far));
                }
            }
            return ret;
        }

        glm::mat4 get_light_space_matrix(const float near_plane, const float far_plane)
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
            glm::vec3 lightDir = lights[0].direction;
            const auto lightView = glm::lookAt(center - lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

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

        void skybox_pass()
        {
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_LEQUAL);
            skybox_shader->bind();
            skybox_shader->set_mat4("projection", &Engine::get_camera()->get_projection_matrix()[0][0]);
            skybox_shader->set_mat4("view", &Engine::get_camera()->get_view_matrix()[0][0]);
            env_cubemap.bind(0);
            render_cube();
            glEnable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
        }

        void probe_debug_pass()
        {
            static bool draw_probes = false;

            if (Input::key_pressed(Input::Key::G))
            {
                draw_probes = !draw_probes;
            }

            if (!draw_probes)
            {
                return;
            }
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            static uint32_t cube_vao = 0;
            static uint32_t cube_vbo = 0;
            if (cube_vao == 0)
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

                glCreateBuffers(1, &cube_vbo);
                glNamedBufferStorage(cube_vbo, sizeof(vertices), vertices, GL_MAP_READ_BIT);
                glCreateVertexArrays(1, &cube_vao);
                glVertexArrayVertexBuffer(cube_vao, 0, cube_vbo, 0, 8 * sizeof(float));

                glEnableVertexArrayAttrib(cube_vao, 0);
                glEnableVertexArrayAttrib(cube_vao, 1);
                glEnableVertexArrayAttrib(cube_vao, 2);

                glVertexArrayAttribFormat(cube_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
                glVertexArrayAttribFormat(cube_vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
                glVertexArrayAttribFormat(cube_vao, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));

                glVertexArrayAttribBinding(cube_vao, 0, 0);
                glVertexArrayAttribBinding(cube_vao, 1, 0);
                glVertexArrayAttribBinding(cube_vao, 2, 0);
            }

            probe_debug_shader->bind();
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, probe_positions);
            probe_debug_shader->set_mat4("projection", glm::value_ptr(Engine::get_camera()->get_projection_matrix()));
            probe_debug_shader->set_mat4("view", glm::value_ptr(Engine::get_camera()->get_view_matrix()));
            probe_debug_shader->set_int("depth", probe_depth);
            probe_debug_shader->set_int("height", probe_height);
            probe_debug_shader->set_int("width", probe_width);
            probe_debug_shader->set_float("spacing", probe_spacing);
            probe_debug_shader->set_vec3("origin", glm::value_ptr(probe_volume_origin));
            glBindVertexArray(cube_vao);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 36, probes.size());
            glDepthMask(GL_TRUE);
            glEnable(GL_BLEND);
        }
    };

    Renderer::~Renderer()
    {
        delete _impl;
    }


    void Renderer::draw_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color)
    {
        _impl->debug_renderer->draw_line(p1, p2, color);
    }

    void Renderer::draw_box(glm::vec3 center, glm::vec3 size, glm::vec3 color)
    {
        _impl->debug_renderer->draw_box(center, size, color);
    }

    void Renderer::draw_sphere(glm::vec3 center, float radius, glm::vec3 color)
    {
        _impl->debug_renderer->draw_sphere(center, radius, color);
    }

    void Renderer::draw_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color)
    {
        _impl->debug_renderer->draw_triangle(p1, p2, p3, color);
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

        if (cologne::Input::key_pressed(Input::Key::H))
        {
            //hot reload shaders
            reload_shaders();
        }

        // _impl->shadow_pass(scene);

        _impl->shadow_pass(scene);
        _impl->gbuffer_pass(scene);
        _impl->lit_pass(scene);
        _impl->skybox_pass();
        _impl->probe_debug_pass();
        _impl->debug_renderer->present();
        _impl->text_renderer->present();
    }

    void Renderer::window_resized(uint32_t width, uint32_t height)
    {
        //regen framebuffers here
        _impl->init_gbuffer(width, height);
        render_scene(*Engine::get_scene());
    }

    void Renderer::reload_shaders()
    {
        _impl->init_shaders();
        _impl->bind_lights(*_impl->lit_shader);
        LOG_INFO("RELOADED SHADERS");
    }

    Shader *Renderer::get_shader_by_name(const char *name)
    {
        const auto n = std::string(name);
        if (!_impl->shaders.contains(n))
        {
            LOG_ERROR("Shader %s not found!", name);
            return nullptr;
        }
        return _impl->shaders[n].get();
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
        return _impl->lights[0];
    }

    void Renderer::set_directional_light(glm::vec3 position, glm::vec3 direction)
    {
        auto &directional_light = get_directional_light();
        directional_light.position = position;
        directional_light.direction = direction;
        reload_shaders();
    }

    Renderer::Renderer()
    {
        _impl = new Impl();
        _impl->init();
        _impl->init_gbuffer(Engine::get_window()->get_width(), Engine::get_window()->get_height());
        _impl->init_shaders();
        glDisable(GL_CULL_FACE);
        _impl->gen_environment_map("newport_loft.hdr");
        _impl->gen_irradiance_map();
        _impl->gen_prefilter_map();
        _impl->gen_brdf_map();
        glEnable(GL_CULL_FACE);
        _impl->add_light(Light(glm::vec3(0.790f, -0.613f, 0.024f), glm::vec3(0.20f, -0.913f, 0.024f),
                               glm::vec3(2.0f, 2.0f, 2.0f), 6.0f, 1.0f,
                               LightType::Directional));
        _impl->add_light(Light(glm::vec3(0.0f, 10.0f, 10.0f), glm::vec3(.0f),
                               glm::vec3(200.0f, 200.0f, 200.0f), 6.0f, 1.0f,
                               LightType::Point));
        _impl->init_shadow_map();
        _impl->create_probe_g_buffers();
    }
}
