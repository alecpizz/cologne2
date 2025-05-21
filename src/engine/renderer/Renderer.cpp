//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"

#include <engine/Engine.h>
#include <engine/Input.h>

#include "DebugRenderer.h"
#include "DebugScope.h"
#include "FrameBuffer.h"
#include "Material.h"
#include "Light.h"
#include "../Scene.h"
#include "Shader.h"
#include "TextRenderer.h"
#include "Probe.h"
#include "../Time.h"

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
    std::shared_ptr<Shader> indirect_shader = nullptr;
    std::shared_ptr<Shader> world_pos_shader = nullptr;
    std::shared_ptr<Shader> mipmap_shader = nullptr;
    std::shared_ptr<Shader> dir_light_shadow_shader = nullptr;
    std::shared_ptr<DebugRenderer> debug_renderer = nullptr;
    std::shared_ptr<TextRenderer> text_renderer = nullptr;
    std::unordered_map<std::string, std::shared_ptr<Shader> > shaders = std::unordered_map<std::string,
        std::shared_ptr<Shader> >();
    std::vector<Light> lights;

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
                                                   RESOURCES_PATH "shaders/voxelize.frag");
        voxelize_debug_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/voxel_visual.vert",
                                                         RESOURCES_PATH "shaders/voxel_visual.frag");

        world_pos_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/world_pos.vert",
                                                    RESOURCES_PATH "shaders/world_pos.frag");
        mipmap_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/mipmap.comp");
        dir_light_shadow_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/dir_shadow.vert",
            RESOURCES_PATH "shaders/dir_shadow.frag");
        indirect_shader = std::make_shared<Shader>(RESOURCES_PATH "shaders/indirect.comp");


        shaders.clear();
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("lit", lit_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("gbuffer", g_buffer_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("skybox", skybox_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("shadowmap", shadowmap_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("probe_debug", probe_debug_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("probe_lit", probe_lit_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("voxelize", voxelize_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("voxelize_debug", voxelize_debug_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("world_pos_shader", world_pos_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("mipmap", mipmap_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader> >("dir_shadow", dir_light_shadow_shader));
        shaders.insert(std::pair<std::string, std::shared_ptr<Shader>>("indirect", indirect_shader));
    }

    void Renderer::add_light(Light light)
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

    void Renderer::update_lights(const Shader &shader)
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

    Renderer::~Renderer()
    {
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

    void draw_fps()
    {
        int fps = 0;
        static int index = 0;
        static float history[30] = {0};
        static float last = 0;
        static float average = 0.0f;
        float fpsFrame = Time::DeltaTime;

        index = (index + 1) % 30;
        average -= history[index];
        history[index] = fpsFrame/30;
        average += history[index];
        fps = static_cast<int>(roundf(1.0f / average));

        text_renderer->draw_text((std::string("FPS: ") +
                                  std::to_string(fps)).c_str(),
                                 glm::vec3(0.0f, 660.0f, 0.0f),
                                 glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), .6f);
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
        indirect_pass();
        lit_pass();
        skybox_pass();
        _output_fbo.blit_to_default_frame_buffer("color", 0, 0,
                                                 Engine::get_window()->get_width(), Engine::get_window()->get_height(),
                                                 GL_COLOR_BUFFER_BIT, GL_NEAREST);
        _output_fbo.release();
        draw_fps();
        debug_voxel_pass();
        debug_renderer->present();
        text_renderer->present();
    }

    void Renderer::window_resized(uint32_t width, uint32_t height)
    {
        //regen framebuffers here
        init_gbuffer();
        init_indirect();
        _voxel_back_fbo.resize(width, height);
        _voxel_front_fbo.resize(width, height);
        _output_fbo.resize(width, height);
        render_scene(*Engine::get_scene());
    }

    void Renderer::reload_shaders()
    {
        init_shaders();
        update_lights(*lit_shader);
        update_shadow(*lit_shader);
        LOG_INFO("RELOADED SHADERS");
    }

    Shader *Renderer::get_shader_by_name(const char *name)
    {
        const auto n = std::string(name);
        if (!shaders.contains(n))
        {
            LOG_ERROR("Shader %s not found!", name);
            return nullptr;
        }
        return shaders[n].get();
    }

    Light &Renderer::get_directional_light() const
    {
        for (auto &light: lights)
        {
            if (light.type == LightType::Directional)
            {
                return light;
            }
        }
        return lights[0];
    }

    void Renderer::set_directional_light(glm::vec3 position, glm::vec3 direction)
    {
        auto &directional_light = get_directional_light();
        directional_light.position = position;
        directional_light.direction = direction;
        update_lights(*lit_shader);
        update_shadow(*lit_shader);
        update_lights(*voxelize_shader);
        voxelize_scene();
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
        DebugScope scope ("initialization");
        init();
        init_shaders();
        init_voxels();
        init_indirect();
        init_gbuffer();
        glDisable(GL_CULL_FACE);
        init_skybox(RESOURCES_PATH "HDR_blue_local_star.hdr");
        init_radiance();
        init_prefilter();
        init_brdf();
        glEnable(GL_CULL_FACE);
        add_light(Light(glm::vec3(0.790f, 18.867f, 0.024f), glm::vec3(0.20f, -0.913f, 0.024f),
                        glm::vec3(2.0f, 2.0f, 2.0f), 6.0f, 1.0f,
                        LightType::Directional));
        add_light(Light(glm::vec3(0.0f, 10.0f, 10.0f), glm::vec3(.0f),
                        glm::vec3(200.0f, 200.0f, 200.0f), 6.0f, 1.0f,
                        LightType::Point));
        init_shadow();
        voxelize_scene();
    }
}
