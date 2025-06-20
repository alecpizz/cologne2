#pragma once

#include "../renderer/types/Model.h"
#include "../renderer/types/Particles.h"
#include <entt/entt.hpp>


namespace cologne
{
    struct TransformComponent;
    class Entity;
    class Editor;
    class Scene
    {
    public:
        Scene();

        ~Scene();

        Model &add_model(const char *path);

        void update(float delta_time);

        Scene(Scene &&) = delete;

        Scene(const Scene &) = delete;

        Scene &operator=(Scene &&) = delete;

        Scene &operator=(const Scene &) = delete;

        AABB re_calculate_bounds();

        AABB get_bounds() const;

        std::vector<Particles> &get_particles();

        Entity create_entity(const std::string& name = std::string());

        void create_static_model_entities(const char* model_name, const TransformComponent &parent_transform, bool create_colliders = false);

        void destroy_entity(Entity entity);

        Entity get_primary_camera();
        Entity get_scene_camera();

        void copy_scene_camera_to_primary_camera();

    private:
        //THIS IS fucking dumb
        AABB _scene_bounds;
        std::vector<Particles> _particles;
        entt::registry _registry;
        friend class Entity;
        friend class Editor;
    };
}
