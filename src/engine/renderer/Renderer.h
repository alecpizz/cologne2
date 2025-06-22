#pragma once
#include "engine/scene/Scene.h"
#include "types/FrameBuffer.h"
#include "engine/renderer/types/RenderItem.h"
#include <filesystem>

namespace cologne
{
    class Scene;
    class Shader;
    class SSBO;
    struct Light;

    class Renderer
    {
        friend class Engine;

    public:
        ~Renderer();

        Renderer(Renderer &&) = delete;

        Renderer(const Renderer &) = delete;

        Renderer &operator=(Renderer &&) = delete;

        Renderer &operator=(const Renderer &) = delete;

        void draw_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color);

        void draw_box(glm::vec3 center, glm::vec3 size, glm::vec3 color);

        void draw_sphere(glm::vec3 center, float radius, glm::vec3 color);

        void draw_triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color);

        void draw_aabb(glm::mat4 transform, glm::vec3 min, glm::vec3 max, glm::vec3 color);

        void draw_text(const char* text, glm::vec3 position, glm::vec4 color, float size);

        void render_scene();

        void window_resized(uint32_t width, uint32_t height);

        void submit_light(Light light);

        void submit_render_item(RenderItem item);
        void submit_skinned_render_item(SkinnedRenderItem item);
        void submit_outline_render_item(RenderItem item);
        void submit_skinned_outline_render_item(SkinnedRenderItem item);

        void reload_shaders();

        Shader *get_shader_by_name(const char *name);

        Light get_directional_light() const;

        void submit_camera_transform(const TransformComponent &tr, CameraComponent cam);

        static glm::mat4 get_camera_view(TransformComponent tr);
        static glm::mat4 get_camera_projection(TransformComponent tr, CameraComponent cam);
        static void file_changed(const std::filesystem::path& path);
        static uint32_t get_output_image();
        static uint32_t read_fbo_pixel(const std::string& fbo_name, const std::string& attachment_name, uint32_t x, uint32_t y);
    private:
        //get me out of here!
        struct VoxelData
        {
            int32_t voxel_dimensions = 256;
        };

        Renderer();

        void init();

        void init_shaders();

        void init_ssbos();

        void update_ssbos();

        void render_cube(int32_t count = 1);

        void render_quad(int32_t count = 1);

        void init_shadow();

        void shadow_pass();

        void update_shadow(Shader &shader);

        void cascaded_shadow_map_pass();

        void dumb_voxel_extra_dir_shadow_pass();

        void point_shadow_pass();

        void init_gbuffer();

        void geometry_pass();

        void lit_pass();

        void init_skybox(const char *hdr_path);

        void init_bloom(uint32_t width = 0, uint32_t height = 0);

        void bloom_pass();

        void skybox_pass();

        void init_framebuffers();

        void init_voxels();

        void init_indirect();

        void init_indirect(uint32_t width, uint32_t height);

        void indirect_pass();

        void voxelize_scene();

        void debug_voxel_pass();

        void init_radiance();

        void init_prefilter();

        void init_brdf();

        void init_outline();

        void outline_pass();

        static FrameBuffer* get_framebuffer_by_name(const char* name);
        SSBO* get_ssbo_by_name(const char* name);

        std::vector<Texture> _shadow_maps;
        std::vector<RenderItem> _render_items;
        std::vector<SkinnedRenderItem> _skinned_render_items;
        std::vector<RenderItem> _outline_render_items;
        std::vector<SkinnedRenderItem> _outline_skinned_render_items;
        std::vector<Light> _lights;

        //Textures
        uint32_t _shadow_depth = 0;
        uint32_t _voxel_texture = 0;
        uint32_t _indirect_texture = 0;
        uint32_t _skybox_texture = 0;
        uint32_t _env_irradiance = 0;
        uint32_t _env_prefilter = 0;
        uint32_t _env_brdf = 0;
        uint32_t _bloom_texture = 0;
        //state
        bool _apply_indirect_lighting = true;
        bool _voxel_debug_visuals = false;
        bool _light_debug_visuals = false;
        //misc
        glm::mat4 _dir_light_space;
        TransformComponent _camera_transform;
        CameraComponent _cam;
        VoxelData _voxel_data;
    };
}
