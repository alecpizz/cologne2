//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"

#include <engine/util/DebugScope.h>

#include "DebugRenderer.h"
#include "engine/core/Engine.h"
#include "engine/core/Input.h"

#include "engine/editor/DebugUI.h"
#include "OpenGLDebugScope.h"
#include "../renderer/types/FrameBuffer.h"
#include "../renderer/types/Light.h"
#include "openglErrorReporting.h"
#include "../scene/Scene.h"
#include "../renderer/types/Shader.h"
#include "TextRenderer.h"
#include "../core/Time.h"
#include "types/SSBO.h"
#include "types/ViewportData.h"

namespace cologne
{
#define IRRADIANCE_INDEX 6
#define PREFILTER_INDEX 7
#define BRDF_INDEX 8

    std::shared_ptr<DebugRenderer> debug_renderer = nullptr;
    std::shared_ptr<TextRenderer> text_renderer = nullptr;
    std::unordered_map<std::string, Shader> shaders = std::unordered_map<std::string, Shader>();
    std::unordered_map<std::string, FrameBuffer> framebuffers = std::unordered_map<std::string, FrameBuffer>();
    std::unordered_map<std::string, SSBO> ssbos = std::unordered_map<std::string, SSBO>();
    std::vector<Light> lights;

    void Renderer::init_shaders()
    {
        shaders.clear();
        shaders["lit"] = Shader(RESOURCES_PATH "shaders/lit.vert",
                                RESOURCES_PATH "shaders/lit.frag");

        shaders["gbuffer"] = Shader(RESOURCES_PATH "shaders/gbuffer.vert",
                                    RESOURCES_PATH "shaders/gbuffer.frag");
        shaders["skinned_gbuffer"] = Shader(RESOURCES_PATH "shaders/skinned_gbuffer.vert",
                                            RESOURCES_PATH "shaders/gbuffer.frag");
        shaders["skybox"] = Shader(RESOURCES_PATH "shaders/skybox.vert",
                                   RESOURCES_PATH "shaders/skybox.frag");
        shaders["shadowmap"] = Shader(RESOURCES_PATH "shaders/shadowmap2.vert",
                                      RESOURCES_PATH "shaders/shadowmap2.frag",
                                      RESOURCES_PATH "shaders/shadowmap2.geom");
        shaders["shadowmap_skinned"] = Shader(RESOURCES_PATH "shaders/shadowmap_skinned.vert",
                                              RESOURCES_PATH "shaders/shadowmap2.frag",
                                              RESOURCES_PATH "shaders/shadowmap2.geom");
        shaders["probe_debug"] = Shader(RESOURCES_PATH "shaders/probe_debug.vert",
                                        RESOURCES_PATH "shaders/probe_debug.frag");
        shaders["probe_lit"] = Shader(RESOURCES_PATH "shaders/probe_lit.comp");

        shaders["voxelize"] = Shader(RESOURCES_PATH "shaders/voxelize.vert",
                                     RESOURCES_PATH "shaders/voxelize.frag");
        shaders["voxelize_debug"] = Shader(RESOURCES_PATH "shaders/voxel_visual.vert",
                                           RESOURCES_PATH "shaders/voxel_visual.frag");

        shaders["world_pos_shader"] = Shader(RESOURCES_PATH "shaders/world_pos.vert",
                                             RESOURCES_PATH "shaders/world_pos.frag");
        shaders["mipmap"] = Shader(RESOURCES_PATH "shaders/mipmap.comp");
        shaders["dir_shadow"] = Shader(RESOURCES_PATH "shaders/dir_shadow.vert",
                                       RESOURCES_PATH "shaders/dir_shadow.frag");
        shaders["indirect"] = Shader(RESOURCES_PATH "shaders/indirect.comp");
        shaders["particle_render"] = Shader(RESOURCES_PATH "shaders/particle_render.vert",
                                            RESOURCES_PATH "shaders/particle_render.frag");
        shaders["particle_sim"] = Shader(RESOURCES_PATH "shaders/particle_sim.comp");

        shaders["downsample"] = Shader(RESOURCES_PATH "shaders/quad.vert",
                                       RESOURCES_PATH "shaders/bloom/downsample.frag");
        shaders["upsample"] = Shader(RESOURCES_PATH "shaders/quad.vert",
            RESOURCES_PATH "shaders/bloom/upsample.frag");
    }

    void Renderer::init_ssbos()
    {
        ssbos["lights"] = SSBO(sizeof(Light) * 8, GL_DYNAMIC_STORAGE_BIT);
        ssbos["viewport"] = SSBO(sizeof(ViewportData), GL_DYNAMIC_STORAGE_BIT);
    }

    void Renderer::update_ssbos()
    {
        OpenGLDebugScope scope("update ssbos");
        ViewportData data{};
        data.projection = get_camera_projection(_camera_transform, _cam);
        data.view = get_camera_view(_camera_transform);
        data.projection_view = data.projection * data.view;
        data.view_inverse = glm::inverse(data.view);
        data.camera_position = glm::vec4(_camera_transform.position, 1.0f);

        ssbos["viewport"].update(sizeof(ViewportData), &data);
        ssbos["viewport"].bind(1);
        ssbos["lights"].update((sizeof(Light) * lights.size()), lights.data());
        ssbos["lights"].bind(2);
    }

    void Renderer::init_framebuffers()
    {
        framebuffers["gbuffer"] = FrameBuffer();
        framebuffers["voxel_front"] = FrameBuffer();
        framebuffers["voxel_back"] = FrameBuffer();
        framebuffers["output"] = FrameBuffer();
        framebuffers["dir_shadow"] = FrameBuffer();
        framebuffers["output"] = FrameBuffer();
    }

    void Renderer::submit_light(Light light)
    {
        lights.emplace_back(light);
    }

    void Renderer::submit_render_item(RenderItem item)
    {
        _render_items.emplace_back(item);
    }

    void Renderer::submit_skinned_render_item(SkinnedRenderItem item)
    {
        _skinned_render_items.emplace_back(item);
    }

    FrameBuffer *Renderer::get_framebuffer_by_name(const char *name)
    {
        const auto n = std::string(name);
        if (!framebuffers.contains(n))
        {
            LOG_ERROR("Framebuffer %s not found!", name);
            return nullptr;
        }
        return &framebuffers[n];
    }

    SSBO *Renderer::get_ssbo_by_name(const char *name)
    {
        const auto n = std::string(name);
        if (!ssbos.contains(n))
        {
            LOG_ERROR("SSBO %s not found!", name);
            return nullptr;
        }
        return &ssbos[n];
    }

    glm::mat4 Renderer::get_camera_view(TransformComponent tr)
    {
        glm::vec3 position = tr.position;
        glm::vec3 fwd = tr.rotation * glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 up = tr.rotation * glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 view_mat = glm::lookAt(position, position + fwd, up);
        return view_mat;
    }

    glm::mat4 Renderer::get_camera_projection(TransformComponent tr, CameraComponent cam)
    {
        return glm::perspective(cam.fov_radians,
                                static_cast<float>(Engine::get_window()->get_width()) /
                                static_cast<float>(Engine::get_window()->get_height()),
                                0.1f, 300.0f);
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

    void Renderer::draw_text(const char *text, glm::vec3 position, glm::vec4 color, float size)
    {
        text_renderer->draw_text(text, position, color, size);
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
        history[index] = fpsFrame / 30;
        average += history[index];
        fps = static_cast<int>(roundf(1.0f / average));

        text_renderer->draw_text((std::string("FPS: ") +
                                  std::to_string(fps)).c_str(),
                                 glm::vec3(0.0f, 660.0f, 0.0f),
                                 glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), .6f);
    }

    void Renderer::render_scene()
    {
        if (cologne::Input::key_pressed(Input::Key::H))
        {
            reload_shaders();
        }
        update_ssbos();
        shadow_pass();
        voxelize_scene();
        geometry_pass();
        indirect_pass();
        bloom_pass();
        lit_pass();
        skybox_pass();
        auto fbo = get_framebuffer_by_name("output");
        fbo->blit_to_default_frame_buffer("color", 0, 0,
                                          Engine::get_window()->get_width(), Engine::get_window()->get_height(),
                                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        fbo->release();
        draw_fps();
        debug_voxel_pass();
        if (_light_debug_visuals)
        {
            for (auto &light: lights)
            {
                draw_sphere(light.position, light.radius, light.color);
            }
        }
        debug_renderer->present();
        text_renderer->present();
        _render_items.clear();
        _skinned_render_items.clear();
        lights.clear();
    }

    void Renderer::window_resized(uint32_t width, uint32_t height)
    {
        //regen framebuffers here
        init_gbuffer();
        init_bloom();
        init_indirect();
        get_framebuffer_by_name("output")->resize(width, height);
        get_framebuffer_by_name("voxel_back")->resize(width, height);
        get_framebuffer_by_name("voxel_front")->resize(width, height);
        // render_scene(*Engine::get_scene());
    }

    void Renderer::reload_shaders()
    {
        init_shaders();
        update_shadow(*get_shader_by_name("lit"));
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
        return &shaders[n];
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

    void Renderer::submit_camera_transform(TransformComponent tr, CameraComponent cam)
    {
        _camera_transform = tr;
        _cam = cam;
    }

    void Renderer::init()
    {
        enableReportGlErrors();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_MULTISAMPLE);
        debug_renderer = std::make_unique<DebugRenderer>();
        text_renderer = std::unique_ptr<TextRenderer>(
            new TextRenderer(RESOURCES_PATH "fonts/Montserrat-Regular.ttf"));

        Engine::get_debug_ui()->add_bool_entry("Voxel Debug Visuals", _voxel_debug_visuals);
        Engine::get_debug_ui()->add_bool_entry("Indirect Lighting", _apply_indirect_lighting);
        Engine::get_debug_ui()->add_bool_entry("Light Debug Visuals", _light_debug_visuals);
        Engine::get_debug_ui()->add_vec3_entry("Voxel Offset", _voxel_data.voxel_offset);
        init_shaders();
        init_ssbos();
        init_bloom();
        init_framebuffers();
        init_voxels();
        init_indirect();
        init_gbuffer();
        glDisable(GL_CULL_FACE);
        init_skybox(RESOURCES_PATH "HDR_blue_local_star.hdr");
        init_radiance();
        init_prefilter();
        init_brdf();
        glEnable(GL_CULL_FACE);

        init_shadow();
        voxelize_scene();
        LOG_INFO("Renderer initialized");
    }

    Renderer::Renderer()
    {
        OpenGLDebugScope scope("initialization");
        DebugScope scope2(__PRETTY_FUNCTION__);
        init();
    }
}
