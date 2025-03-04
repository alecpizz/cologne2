#pragma once

namespace goon
{
    class Mesh;

    class Model
    {
    public:
        explicit Model(const char *path);

        ~Model();

        Model(Model &&) = delete;

        Model &operator=(Model &&) = delete;

        Model(const Model &) = delete;

        Model &operator=(const Model &) = delete;

        Mesh *get_meshes() const;

        uint32_t get_num_meshes() const;

    private:
        struct Impl;
        Impl *_impl;
    };
}
