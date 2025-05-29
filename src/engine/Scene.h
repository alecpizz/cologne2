#pragma once
#include "Animator.h"
#include "renderer/Model.h"
#include "renderer/Particles.h"
#include "renderer/SkinnedModel.h"

namespace cologne
{
    class Scene
    {
    public:
        Scene();

        ~Scene();

        Model *get_model_by_index(size_t idx) const;

        uint64_t get_model_count() const;

        Model &add_model(const char *path, bool flip_textures);

        SkinnedModel &add_skinned_model(const char* path, const char* name);

        std::vector<SkinnedModel>& get_skinned_models();
        std::unordered_map<std::string, Animator>& get_animators();

        void update(float delta_time);

        Scene(Scene &&) = delete;

        Scene(const Scene &) = delete;

        Scene &operator=(Scene &&) = delete;

        Scene &operator=(const Scene &) = delete;

        AABB re_calculate_bounds();

        AABB get_bounds() const;

        std::vector<Particles> &get_particles();

    private:
        //THIS IS fucking dumb
        AABB _scene_bounds;
        std::vector<std::unique_ptr<Model> > _models;
        std::vector<SkinnedModel> _skinned_models;
        std::unordered_map<std::string, Animator> _animators;
        std::vector<Animation> _animations;
        std::vector<Particles> _particles;
    };
}
