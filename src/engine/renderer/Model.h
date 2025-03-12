#pragma once

#include "Texture.h"
#include "Mesh.h"

namespace goon
{
    class Model
    {
    public:
        explicit Model(const char *path);

        ~Model();

        Texture* get_albedo() const;
        Texture* get_normal() const;
        Texture* get_ao() const;
        Texture* get_roughness() const;
        Texture* get_metallic() const;

        void set_texture(const char* path, TextureType type);
        // Model(Model &&) = delete;

        // Model &operator=(Model &&) = delete;

        // Model(const Model &) = delete;

        // Model &operator=(const Model &) = delete;

        Mesh *get_meshes();

        uint64_t get_num_meshes() const;

    private:
        struct Impl;
        Impl * _impl;
    };
}
