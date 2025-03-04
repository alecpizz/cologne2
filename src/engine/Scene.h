#pragma once

class Model;

namespace goon
{
    class Scene
    {
    public:
        Scene();

        ~Scene();

        Model* get_models() const;

        uint64_t get_model_count() const;

        Scene(Scene &&) = delete;

        Scene(const Scene &) = delete;

        Scene &operator=(Scene &&) = delete;

        Scene &operator=(const Scene &) = delete;

    private:
        struct Impl;
        Impl *_impl;

    };
}
