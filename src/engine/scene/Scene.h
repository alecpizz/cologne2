#pragma once
#include "../animation/Animator.h"
#include "../renderer/types/Model.h"
#include "../renderer/types/Particles.h"
#include "../renderer/types/SkinnedModel.h"
#include <entt/entt.hpp>



namespace cologne
{
    class Entity;
    class DebugUI;
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

        void destroy_entity(Entity entity);

        Entity get_camera_entity();

    private:
        //THIS IS fucking dumb
        AABB _scene_bounds;
        std::vector<Particles> _particles;
        entt::registry _registry;
        friend class Entity;
        friend class DebugUI;
    };
}
