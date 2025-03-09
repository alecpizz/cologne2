#pragma once

#include "Texture.h"
#include "Mesh.h""

namespace goon
{
    class Model
    {
    public:
        explicit Model(const char *path);

        ~Model();

        // Model(Model &&) = delete;

        // Model &operator=(Model &&) = delete;

        // Model(const Model &) = delete;

        // Model &operator=(const Model &) = delete;

        Mesh *get_meshes();

        uint64_t get_num_meshes() const;

    private:
        std::vector<Mesh> _meshes;
        std::string _directory;
        std::vector<Texture> _textures;
    };
}
