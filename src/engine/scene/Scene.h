#pragma once

#include "../renderer/types/Particles.h"
#include <entt/entt.hpp>
#include <engine/core/UUID.h>

namespace cologne
{
    class System;
}

namespace cologne
{
    struct TransformComponent;
    class Entity;
    class Editor;

    class Scene
    {
    public:
        Scene();

        explicit Scene(const char *path);

        ~Scene();

        void update_runtime(float delta_time);
        void update_editor(float delta_time);

        void on_enter_play_mode();
        void on_exit_play_mode();



        void on_enter_edit_mode();
        void on_exit_edit_mode();

        Scene(Scene &&) = delete;

        Scene(const Scene &) = delete;

        Scene &operator=(Scene &&) = delete;

        Scene &operator=(const Scene &) = delete;

        static Ref<Scene> copy(Ref<Scene> scene);

        AABB re_calculate_bounds();

        AABB get_bounds() const;

        std::vector<Particles> &get_particles();

        Entity create_entity_with_uuid(UUID id, const std::string &name = std::string());

        Entity create_entity(const std::string &name = std::string());

        Entity get_entity_by_uuid(UUID uuid);

        Entity create_static_model_entities(const char *model_name, const TransformComponent &parent_transform,
                                            bool create_colliders = false);


        void destroy_entity(Entity entity);

        Entity get_primary_camera();

        Entity get_scene_camera();

        Entity duplicate_entity(Entity source);

        void copy_scene_camera_to_primary_camera();

        void create_bullet(glm::vec3 pos, glm::vec3 dir, float damage);

        const std::string &get_scene_name() const { return _scene_name; }
        void set_scene_name(const std::string &path) { _scene_name = path; }
        entt::registry& get_raw_registry() { return _registry;}
        void setup_blank_scene();
        static void initialize_systems();
    private:
        static void add_system(std::unique_ptr<System> system);
        void duplicate_recursive(Entity source, std::unordered_map<UUID, UUID>& old_to_new_map);
        void initialize_physics_world();
        Entity create_player_entity(glm::vec3 pos);
        Entity create_scene_camera_entity();
        void setup_entity_map();

        std::string _scene_name = "untitled_scene.cscn";
        AABB _scene_bounds;
        std::vector<std::unique_ptr<System>> _editor_system;
        std::vector<Particles> _particles;
        std::unordered_map<UUID, entt::entity> _entity_map;
        entt::registry _registry;
        friend class Entity;
        friend class Editor;
        friend class SceneSaver;
    };
}
