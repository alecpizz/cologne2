#pragma once

#include "Texture.h"
#include "Mesh.h"
#include "../Transform.h"
#include "Material.h"

namespace goon
{
    class Model
    {
    public:
        Model(const char *path, bool flip_textures);

        ~Model();


        Transform *get_transform() const;

        // Model(Model &&) = delete;

        // Model &operator=(Model &&) = delete;

        // Model(const Model &) = delete;

        // Model &operator=(const Model &) = delete;
        const char *get_path() const;

        Material *get_materials() const;

        uint64_t get_num_materials() const;

        Mesh *get_meshes();

        uint64_t get_num_meshes() const;

        void draw() const;

    private:
        Transform *_transform = nullptr;
        struct Impl;
        Impl *_impl;
    };
}
