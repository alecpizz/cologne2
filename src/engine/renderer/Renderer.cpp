//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/util/DebugScope.h>

#include "DebugRenderer.h"
#include "engine/core/Engine.h"
#include "engine/core/Input.h"

#include "engine/editor/Editor.h"
#include "OpenGLDebugScope.h"
#include "../renderer/types/FrameBuffer.h"
#include "openglErrorReporting.h"
#include "../scene/Scene.h"
#include "../renderer/types/Shader.h"
#include "TextRenderer.h"
#include "../core/Time.h"
#include "types/SSBO.h"
#include "types/ViewportData.h"
#include <engine/renderer/types/Light.h>

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

    void Renderer::init_shaders()
    {
        for (auto &shader: shaders | std::views::values)
        {
            shader.cleanup();
        }
        if (!shaders.empty())
        {
            shaders.clear();
        }
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
        // shaders["probe_lit"] = Shader(RESOURCES_PATH "shaders/probe_lit.comp");

        shaders["voxelize"] = Shader(RESOURCES_PATH "shaders/voxelize.vert",
                                     RESOURCES_PATH "shaders/voxelize.frag");
        shaders["voxelize_debug"] = Shader(RESOURCES_PATH "shaders/vxgi/voxel_debug.comp");

        shaders["world_pos_shader"] = Shader(RESOURCES_PATH "shaders/world_pos.vert",
                                             RESOURCES_PATH "shaders/world_pos.frag");
        shaders["mipmap"] = Shader(RESOURCES_PATH "shaders/mipmap.comp");
        shaders["dir_shadow"] = Shader(RESOURCES_PATH "shaders/shadows/dir_shadow.vert",
                                       RESOURCES_PATH "shaders/shadows/dir_shadow.frag");
        shaders["indirect"] = Shader(RESOURCES_PATH "shaders/indirect.comp");
        shaders["particle_render"] = Shader(RESOURCES_PATH "shaders/particle_render.vert",
                                            RESOURCES_PATH "shaders/particle_render.frag");
        shaders["particle_sim"] = Shader(RESOURCES_PATH "shaders/particle_sim.comp");

        //bloom
        shaders["downsample"] = Shader(RESOURCES_PATH "shaders/quad.vert",
                                       RESOURCES_PATH "shaders/bloom/downsample.frag");
        shaders["upsample"] = Shader(RESOURCES_PATH "shaders/quad.vert",
                                     RESOURCES_PATH "shaders/bloom/upsample.frag");

        shaders["point_shadow"] = Shader(RESOURCES_PATH "shaders/shadows/point_shadow.vert",
                                         RESOURCES_PATH "shaders/shadows/point_shadow.frag");
        //outline
        shaders["outline_mask"] = Shader(RESOURCES_PATH "shaders/outline/outline_mask.vert",
                                         RESOURCES_PATH "shaders/outline/outline_mask.frag");
        shaders["outline"] = Shader(RESOURCES_PATH "shaders/outline/outline.vert",
                                    RESOURCES_PATH "shaders/outline/outline.frag");
        shaders["outline_composite"] = Shader(RESOURCES_PATH "shaders/outline/outline_composite.comp");
        shaders["indirect_upsample"] = Shader(RESOURCES_PATH "shaders/vxgi/indirect_upsample.comp");
    }

    void Renderer::init_ssbos()
    {
        ssbos["lights"] = SSBO(sizeof(Light) * 8, GL_DYNAMIC_DRAW);
        ssbos["viewport"] = SSBO(sizeof(ViewportData), GL_DYNAMIC_DRAW);
        ssbos["model_matrices"] = SSBO(sizeof(glm::mat4) * 128, GL_DYNAMIC_DRAW);
        ssbos["draw_cmds"] = SSBO(sizeof(MultiDrawElementsCommand) * 128, GL_DYNAMIC_DRAW);
        ssbos["materials"] = SSBO(sizeof(GPUMaterial) * 128, GL_DYNAMIC_DRAW);
        ssbos["skinning_transforms"] = SSBO(sizeof(glm::mat4) * 2048, GL_DYNAMIC_DRAW);
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
        data.projection_view_inverse = glm::inverse(data.projection_view);
        static std::vector<MultiDrawElementsCommand> draw_cmds;
        static std::vector<glm::mat4> model_matrices;
        static std::vector<GPUMaterial> gpu_materials;
        draw_cmds.clear();
        gpu_materials.clear();
        model_matrices.clear();
        for (auto& render_item : _render_items)
        {
            MultiDrawElementsCommand cmd;
            //todo: remove extra index step
            auto mesh = AssetManager::get_mesh_by_index(render_item.mesh_idx);
            if (!mesh)
            {
                continue;
            }
            cmd.index_count = mesh->get_indices().size();
            cmd.instance_count = 1;
            cmd.first_index = mesh->get_first_index();
            cmd.base_vertex = mesh->get_base_vertex();
            cmd.base_instance = render_item.entity_id;

            draw_cmds.emplace_back(cmd);
            model_matrices.emplace_back(render_item.transform);

            //todo: also remove this extra index step
            auto mat = AssetManager::get_material_by_index(mesh->get_material_index());
            if (mat)
            {
                mat->ensure_bindless();
                gpu_materials.emplace_back(mat->to_gpu_material());
            }
            else
            {
                gpu_materials.emplace_back();
            }
        }
        ssbos["model_matrices"].update(sizeof(glm::mat4) * model_matrices.size(), model_matrices.data());
        ssbos["draw_cmds"].update(sizeof(MultiDrawElementsCommand) * draw_cmds.size(), draw_cmds.data());
        ssbos["lights"].update(sizeof(Light) * _lights.size(), _lights.data());
        ssbos["viewport"].update(sizeof(ViewportData), &data);
        ssbos["materials"].update(sizeof(GPUMaterial) * gpu_materials.size(), gpu_materials.data());

        ssbos["viewport"].bind(1);
        ssbos["lights"].bind(2);
        ssbos["draw_cmds"].bind(3);
        ssbos["model_matrices"].bind(4);
        ssbos["materials"].bind(5);
    }

    void Renderer::init_framebuffers()
    {
        framebuffers["gbuffer"] = FrameBuffer();
        framebuffers["voxel_front"] = FrameBuffer();
        framebuffers["voxel_back"] = FrameBuffer();
        framebuffers["output"] = FrameBuffer();
        framebuffers["output"] = FrameBuffer();
        framebuffers["outline"] = FrameBuffer();
    }

    void Renderer::submit_light(Light light)
    {
        _lights.emplace_back(light);
    }

    void Renderer::submit_render_item(RenderItem item)
    {
        _render_items.emplace_back(item);
    }

    void Renderer::submit_skinned_render_item(SkinnedRenderItem item)
    {
        _skinned_render_items.emplace_back(item);
    }

    void Renderer::submit_outline_render_item(RenderItem item)
    {
        _outline_render_items.emplace_back(item);
    }

    void Renderer::submit_skinned_outline_render_item(SkinnedRenderItem item)
    {
        _outline_skinned_render_items.emplace_back(item);
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

    void Renderer::file_changed(const std::filesystem::path &path)
    {
        if (path.has_extension() && path.extension().string() == ".vert" || path.extension().string() == ".frag" || path
            .extension().string() == ".comp" || path.extension().string() == ".geom")
        {
            Engine::get_renderer()->reload_shaders();
        }
    }

    uint32_t Renderer::get_output_image()
    {
        return get_framebuffer_by_name("output")->get_color_attachment_handle_by_name("color");
    }

    uint32_t Renderer::read_fbo_pixel(const std::string &fbo_name, const std::string &attachment_name, uint32_t x,
                                      uint32_t y)
    {
        auto fbo = get_framebuffer_by_name(fbo_name.c_str());
        if (!fbo)
        {
            LOG_ERROR("No framebuffeer with name %s", fbo_name.c_str());
            return entt::null;
        }
        fbo->bind();
        glReadBuffer(fbo->get_color_attachment_slot_by_name(attachment_name.c_str()));
        uint32_t pixelData;
        glReadPixels(x, y, 1, 1, fbo->get_color_attachment_format_by_name(attachment_name.c_str()), GL_UNSIGNED_INT,
                     &pixelData);
        fbo->release();
        return pixelData;
    }

    void Renderer::upload_vertex_data(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
    {
        if (_vertex_data_vao != 0)
        {
            glDeleteVertexArrays(1, &_vertex_data_vao);
            glDeleteBuffers(1, &_vertex_data_ebo);
            glDeleteBuffers(1, &_vertex_data_vbo);
        }

        glCreateBuffers(1, &_vertex_data_vbo);
        glNamedBufferStorage(_vertex_data_vbo, sizeof(Vertex) * vertices.size(), vertices.data(),
            GL_MAP_READ_BIT);

        glCreateBuffers(1, &_vertex_data_ebo);
        glNamedBufferStorage(_vertex_data_ebo, sizeof(uint32_t) * indices.size(), indices.data(),
            GL_MAP_READ_BIT);

        glCreateVertexArrays(1, &_vertex_data_vao);

        glVertexArrayVertexBuffer(_vertex_data_vao, 0, _vertex_data_vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(_vertex_data_vao, _vertex_data_ebo);

        glEnableVertexArrayAttrib(_vertex_data_vao, 0);
        glEnableVertexArrayAttrib(_vertex_data_vao, 1);
        glEnableVertexArrayAttrib(_vertex_data_vao, 2);
        glEnableVertexArrayAttrib(_vertex_data_vao, 3);

        glVertexArrayAttribFormat(_vertex_data_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribFormat(_vertex_data_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribFormat(_vertex_data_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
        glVertexArrayAttribFormat(_vertex_data_vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent));

        glVertexArrayAttribBinding(_vertex_data_vao, 0, 0);
        glVertexArrayAttribBinding(_vertex_data_vao, 1, 0);
        glVertexArrayAttribBinding(_vertex_data_vao, 2, 0);
        glVertexArrayAttribBinding(_vertex_data_vao, 3, 0);
    }

    void Renderer::upload_weighted_vertex_data(std::vector<WeightedVertex> &vertices, std::vector<uint32_t> &indices)
    {
        if (_skinned_bind_pose_vbo != 0)
        {
            glDeleteBuffers(1, &_skinned_bind_pose_vbo);
            glDeleteBuffers(1, &_skinned_bind_pose_ebo);
        }

        glCreateBuffers(1, &_skinned_bind_pose_vbo);
        glNamedBufferStorage(_skinned_bind_pose_vbo, sizeof(WeightedVertex) * vertices.size(), vertices.data(),
            GL_MAP_READ_BIT);

        glCreateBuffers(1, &_skinned_bind_pose_ebo);
        glNamedBufferStorage(_skinned_bind_pose_ebo, sizeof(uint32_t) * indices.size(), indices.data(),
            GL_MAP_READ_BIT);
    }

    void Renderer::allocate_weighted_vertex_buffer(size_t count)
    {
        if (_skinned_vao == 0)
        {
            glCreateVertexArrays(1, &_skinned_vao);
        }
        if (_skinned_vbo_size < count * sizeof(Vertex))
        {
            if (_skinned_vbo != 0)
            {
                glDeleteBuffers(1, &_skinned_vbo);
            }

            glCreateBuffers(1, &_skinned_vbo);
            glNamedBufferStorage(_skinned_vbo, sizeof(Vertex) * count, nullptr, GL_MAP_READ_BIT);

            glCreateVertexArrays(1, &_skinned_vao);

            glVertexArrayVertexBuffer(_skinned_vao, 0, _skinned_vbo, 0, sizeof(Vertex));
            glVertexArrayElementBuffer(_skinned_vao, _skinned_bind_pose_ebo);

            glEnableVertexArrayAttrib(_skinned_vao, 0);
            glEnableVertexArrayAttrib(_skinned_vao, 1);
            glEnableVertexArrayAttrib(_skinned_vao, 2);
            glEnableVertexArrayAttrib(_skinned_vao, 3);

            glVertexArrayAttribFormat(_skinned_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
            glVertexArrayAttribFormat(_skinned_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
            glVertexArrayAttribFormat(_skinned_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
            glVertexArrayAttribFormat(_skinned_vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent));

            glVertexArrayAttribBinding(_skinned_vao, 0, 0);
            glVertexArrayAttribBinding(_skinned_vao, 1, 0);
            glVertexArrayAttribBinding(_skinned_vao, 2, 0);
            glVertexArrayAttribBinding(_skinned_vao, 3, 0);
        }
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
        compute_skinning_pass();
        shadow_pass();
        voxelize_scene();
        geometry_pass();
        skybox_pass();
        indirect_pass();
        bloom_pass();
        lit_pass();
        draw_fps();
        outline_pass();
        auto fbo = get_framebuffer_by_name("output");
        fbo->bind();
        debug_voxel_pass();
        debug_renderer->present();
        text_renderer->present();
        fbo->blit_to_default_frame_buffer("color", 0, 0,
                                          Engine::get_window()->get_width(), Engine::get_window()->get_height(),
                                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        fbo->release();
        _render_items.clear();
        _skinned_render_items.clear();
        _lights.clear();
        _outline_render_items.clear();
        _outline_skinned_render_items.clear();
    }

    void Renderer::window_resized(uint32_t width, uint32_t height)
    {
        //regen framebuffers here
        init_bloom();
        init_indirect(width, height);
        get_framebuffer_by_name("gbuffer")->resize(width, height);
        get_framebuffer_by_name("output")->resize(width, height);
        get_framebuffer_by_name("voxel_back")->resize(width, height);
        get_framebuffer_by_name("voxel_front")->resize(width, height);
        get_framebuffer_by_name("outline")->resize(width, height);
        // render_scene(*Engine::get_scene());
    }

    void Renderer::reload_shaders()
    {
        init_shaders();
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

    void Renderer::submit_camera_transform(const TransformComponent &tr, const CameraComponent cam)
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
        init_shaders();
        init_ssbos();
        init_bloom();
        init_framebuffers();
        init_voxels();
        init_indirect();
        init_gbuffer();
        init_outline();
        glDisable(GL_CULL_FACE);
        init_skybox(RESOURCES_PATH "HDR_blue_local_star.hdr");
        init_radiance();
        init_prefilter();
        init_brdf();
        glEnable(GL_CULL_FACE);

        init_shadow();
        LOG_INFO("Renderer initialized");
    }

    Renderer::Renderer()
    {
        OpenGLDebugScope scope("initialization");
        DebugScope scope2(__PRETTY_FUNCTION__);
        init();
    }
}
