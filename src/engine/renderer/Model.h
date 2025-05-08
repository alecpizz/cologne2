#pragma once

#include "Texture.h"
#include "Mesh.h"
#include "../Transform.h"
#include "Material.h"
#include "../AABB.h"

namespace cologne
{
    class Model
    {
    public:
        Model(const char *path, bool flip_textures);

        ~Model();


        Transform *get_transform() const;
        AABB get_aabb() const;

        const char *get_path() const;

        Material *get_materials() const;

        uint64_t get_num_materials() const;

        Mesh *get_meshes();

        uint64_t get_num_meshes() const;

        void set_active(bool active);

        void set_aabb(AABB aabb);

        bool get_active() const;

    private:
        bool _active = true;
        Transform *_transform = nullptr;
        AABB _bounds;
        struct Impl;
        Impl *_impl;
    };
}
