#pragma once

#include <engine/Types.h>

#include "Texture.h"
#include "Mesh.h"
#include "engine/Transform.h"
#include "Material.h"
#include "engine/AABB.h"


namespace cologne
{
    class Model
    {
    public:
        Model() = default;

        explicit Model(const ModelData& data);

        ~Model();

        Transform &get_transform();

        AABB get_aabb() const;

        const char *get_path() const;

        Material *get_materials();

        uint64_t get_num_materials() const;

        Mesh *get_meshes();

        uint64_t get_num_meshes() const;

        void set_active(bool active);

        void set_aabb(AABB aabb);

        bool get_active() const;

        bool get_gi_only() const;

        bool get_cast_shadows() const;

        void set_cast_shadows(bool b);

        void set_gi_only(bool b);

    private:
        std::vector<Mesh> _meshes = std::vector<Mesh>();
        std::vector<Material> _materials = std::vector<Material>();
        bool _active = true;
        bool _cast_shadows = true;
        bool _gi_only = false;
        std::string _path;
        Transform _transform;
        AABB _bounds;
    };
}
