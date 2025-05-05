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


    std::shared_ptr<Shader> lit_shader = nullptr;
    std::shared_ptr<Shader> g_buffer_shader = nullptr;
    std::shared_ptr<Shader> skybox_shader = nullptr;
    std::shared_ptr<Shader> shadowmap_shader = nullptr;
    std::shared_ptr<Shader> fbo_debug_shader = nullptr;
    std::shared_ptr<Shader> probe_debug_shader = nullptr;
    std::shared_ptr<Shader> probe_lit_shader = nullptr;
    std::shared_ptr<Shader> voxelize_shader = nullptr;
    std::shared_ptr<Shader> voxelize_debug_shader = nullptr;
    std::shared_ptr<Shader> world_pos_shader = nullptr;
    std::shared_ptr<Shader> mipmap_shader = nullptr;
    std::shared_ptr<DebugRenderer> debug_renderer = nullptr;
    std::shared_ptr<TextRenderer> text_renderer = nullptr;
    std::unordered_map<std::string, std::shared_ptr<Shader> > shaders = std::unordered_map<std::string,
        std::shared_ptr<Shader> >();
    std::vector<Light> lights;

    struct Renderer::Impl
    {
        Texture env_cubemap;
        Texture env_irradiance;
        Texture env_prefilter;
        Texture env_brdf;

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
            voxelize_scene();
        }


        void bind_lights(const Shader &shader)
        {
            shader.bind();
            for (size_t i = 0; i < lights.size(); i++)
            {
                shader.set_vec3(std::string("lights[" + std::to_string(i) + "].position").c_str(),
                                glm::value_ptr(lights[i].position));
                shader.set_vec3(std::string("lights[" + std::to_string(i) + "].color").c_str(),
                                glm::value_ptr(lights[i].color));
                shader.set_float(std::string("lights[" + std::to_string(i) + "].radius").c_str(),
                                 lights[i].radius);
                shader.set_int(std::string("lights[" + std::to_string(i) + "].type").c_str(),
                               lights[i].type);
                shader.set_float(std::string("lights[" + std::to_string(i) + "].strength").c_str(),
                                 lights[i].strength);
                shader.set_vec3(std::string("lights[" + std::to_string(i) + "].direction").c_str(),
                                glm::value_ptr(lights[i].direction));
            }
            shader.set_int("num_lights", lights.size());
        }


        void debug_voxel_pass()
        {
            if (!voxel_debug_visuals)
            {
                return;
            }

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
            glBindFramebuffer(GL_FRAMEBUFFER, voxel_cube_back_fbo);
            glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            render_cube();

            glCullFace(GL_BACK);
            glBindFramebuffer(GL_FRAMEBUFFER, voxel_cube_front_fbo);
            glViewport(0, 0, Engine::get_window()->get_width(), Engine::get_window()->get_height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            render_cube();

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

            glBindTextureUnit(0, voxel_texture);
            glBindTextureUnit(1, voxel_cube_back);
            glBindTextureUnit(2, voxel_cube_front);
            render_quad();

            glEnable(GL_DEPTH_TEST);
        }
    };

    Renderer::~Renderer()
    {
        delete _impl;
    }


    void Renderer::draw_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color)
    {
        debug_renderer->draw_line(p1, p2, color);
    }

    void Renderer::draw_box(glm::vec3 center, glm::vec3 size, glm::vec3 color)
    {
        debug_renderer->draw_box(center, size, color);
    }

    void Renderer::draw_sphere(glm::vec3 center, float radius, glm::vec3 color)
    {
        debug_renderer->draw_sphere(center, radius, color);
    }

    void Renderer::draw_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color)
    {
        debug_renderer->draw_triangle(p1, p2, p3, color);
    }

    void Renderer::draw_aabb(glm::mat4 transform, glm::vec3 min, glm::vec3 max, glm::vec3 color)
    {
        debug_renderer->draw_aabb(transform, min, max, color);
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
            reload_shaders();
        }
        shadow_pass(scene);
        voxelize_scene();
        geometry_pass(scene);
        lit_pass();
        skybox_pass();
        _impl->debug_voxel_pass();
        debug_renderer->present();
        text_renderer->present();
    }

    void Renderer::window_resized(uint32_t width, uint32_t height)
    {
        //regen framebuffers here
        init_gbuffer();
        _impl->init_voxel_fbo();
        render_scene(*Engine::get_scene());
    }

    void Renderer::reload_shaders()
    {
        init_shaders();
        _impl->bind_lights(*_impl->lit_shader);
        update_shadow(*lit_shader);
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
        _impl->bind_lights(*_impl->lit_shader);
        update_shadow(*_impl->lit_shader);
        _impl->bind_lights(*_impl->voxelize_shader);
        voxelize_scene();
    }

    void Renderer::init_shaders()
    {
        lit_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/lit.vert",
                                              RESOURCES_PATH "shaders/lit.frag");

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

        voxelize_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/voxelize.vert",
                                                   RESOURCES_PATH "shaders/voxelize.frag",
                                                   RESOURCES_PATH "shaders/voxelize.geom");
        voxelize_debug_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/voxel_visual.vert",
                                                         RESOURCES_PATH "shaders/voxel_visual.frag");

        world_pos_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/world_pos.vert",
                                                    RESOURCES_PATH "shaders/world_pos.frag");
        mipmap_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/mipmap.comp");

        shaders.clear();
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("lit", lit_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("gbuffer", g_buffer_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("skybox", skybox_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("shadowmap", shadowmap_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("probe_debug", probe_debug_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("probe_lit", probe_lit_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("voxelize", voxelize_shader));
    }

    void Renderer::init()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_MULTISAMPLE);
        debug_renderer = std::make_unique<DebugRenderer>();
        text_renderer = std::unique_ptr<TextRenderer>(
            new TextRenderer(RESOURCES_PATH "fonts/Montserrat-Regular.ttf"));

        Engine::get_debug_ui()->add_bool_entry("Voxel Debug Visuals", _voxel_debug_visuals);
        Engine::get_debug_ui()->add_bool_entry("Indirect Lighting", _apply_indirect_lighting);
        Engine::get_debug_ui()->add_vec3_entry("Voxel Offset", _voxel_data.voxel_offset);
    }

    Renderer::Renderer()
    {
        _impl = new Impl();
        init();
        init_shaders();
        init_voxels();
        init_gbuffer();
        glDisable(GL_CULL_FACE);
        init_skybox("HDR_blue_local_star.hdr");
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
        init_shadow();
        voxelize_scene();
    }
}
