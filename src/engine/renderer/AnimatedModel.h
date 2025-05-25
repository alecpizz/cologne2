//
// Created by alecpizz on 5/25/2025.
//

#pragma once
#include <engine/AABB.h>
#include <engine/Transform.h>

#include "Material.h"
#include "Mesh.h"

namespace cologne
{
    class AnimatedModel
    {
    public:
        explicit AnimatedModel(const char *path);

        ~AnimatedModel();

        Transform &get_transform();

        AABB get_aabb() const;

        Material *get_materials();

        uint64_t get_num_materials() const;

        Mesh *get_meshes();

        uint64_t get_num_meshes() const;

        void set_active(bool active);

        void set_aabb(AABB aabb);

        bool get_active() const;

    private:
        std::vector<Material> _materials;
        std::vector<Mesh> _meshes;
        bool _active = true;
        Transform _transform;
        AABB _bounds;
    };
}
