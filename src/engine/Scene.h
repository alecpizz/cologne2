#pragma once
#include "renderer/Model.h"

namespace cologne
{
    class Scene
    {
    public:
        Scene();

        ~Scene();

        Model* get_model_by_index(size_t idx) const;

        uint64_t get_model_count() const;

        Model& add_model(const char* path, bool flip_textures) const;

        void update(float delta_time);

        Scene(Scene &&) = delete;

        Scene(const Scene &) = delete;

        Scene &operator=(Scene &&) = delete;

        Scene &operator=(const Scene &) = delete;

    private:
        struct Impl;
        Impl *_impl;

    };
}
