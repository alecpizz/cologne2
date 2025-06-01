#pragma once
#include "../animation/Animator.h"
#include "../renderer/types/Model.h"
#include "../renderer/types/Particles.h"
#include "../renderer/types/SkinnedModel.h"
#include <entt/entt.hpp>


namespace cologne
{
    class Entity;
    class Scene
    {
    public:
        Scene();

        ~Scene();

        Model &add_model(const char *path);

        SkinnedModel &add_skinned_model(const char *path);

        std::vector<SkinnedModel> &get_skinned_models();
        std::vector<Model> &get_models();

        std::unordered_map<std::string, Animator> &get_animators();

        void update(float delta_time);

        Scene(Scene &&) = delete;

        Scene(const Scene &) = delete;

        Scene &operator=(Scene &&) = delete;

        Scene &operator=(const Scene &) = delete;

        AABB re_calculate_bounds();

        AABB get_bounds() const;

        std::vector<Particles> &get_particles();

        Entity create_entity(const std::string& name = std::string());

    private:
        //THIS IS fucking dumb
        AABB _scene_bounds;
        std::vector<Model> _models;
        std::vector<SkinnedModel> _skinned_models;
        std::unordered_map<std::string, Animator> _animators;
        std::vector<Animation> _animations;
        std::vector<Particles> _particles;
        entt::registry _registry;
        friend class Entity;
    };
}
