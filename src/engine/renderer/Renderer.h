#pragma once
#include "engine/renderer/types/Light.h"
#include "engine/scene/Scene.h"
#include "types/FrameBuffer.h"
#include "engine/renderer/types/RenderItem.h"


namespace cologne
{
    class Scene;
    class Shader;

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

        void render_scene(Scene &scene);

        void window_resized(uint32_t width, uint32_t height);

        void add_light(Light light);

        void submit_render_item(RenderItem item);

        void reload_shaders();

        Shader *get_shader_by_name(const char *name);

        Light &get_directional_light() const;

        void set_directional_light(glm::vec3 position, glm::vec3 direction);

    private:
        //get me out of here!
        struct VoxelData
        {
            int32_t voxel_dimensions = 256;
            glm::vec3 voxel_offset = glm::vec3(0.0f, -0.425f, 0.0f);
        };

        Renderer();

        void init();

        void init_shaders();

        void render_cube(int32_t count = 1);

        void render_quad();

        void init_shadow();

        void shadow_pass(Scene &scene);

        void update_shadow(Shader &shader);

        void init_gbuffer();

        void geometry_pass(Scene &scene);

        void lit_pass();

        void init_skybox(const char *hdr_path);

        void skybox_pass();

        void init_framebuffers();

        void init_voxels();

        void init_indirect();

        void indirect_pass();

        void voxelize_scene();

        void debug_voxel_pass();

        void init_radiance();

        void init_prefilter();

        void init_brdf();

        void update_lights(Shader &shader);

        FrameBuffer* get_framebuffer_by_name(const char* name);

        std::vector<RenderItem> _render_items;

        //Textures
        uint32_t _shadow_depth;
        uint32_t _voxel_texture;
        uint32_t _indirect_texture;
        uint32_t _skybox_texture;
        uint32_t _env_irradiance;
        uint32_t _env_prefilter;
        uint32_t _env_brdf;
        //state
        bool _apply_indirect_lighting = true;
        bool _voxel_debug_visuals = false;
        //misc
        glm::mat4 _dir_light_space;
        VoxelData _voxel_data;
    };
}
