#pragma once

#include "Texture.h"
#include "Mesh.h"
#include "engine/Transform.h"
#include "Material.h"
#include "engine/AABB.h"

struct aiMesh;
struct aiNode;
struct aiScene;

namespace cologne
{
    class Model
    {
    public:
        Model() = default;

        Model(const char *path, bool flip_textures);

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

        void load_model();

        void load_materials(const aiScene *scene);

        void process_node(const aiNode *node, const aiScene *scene);

        Mesh process_mesh(aiMesh *mesh);

        bool _active = true;
        bool _cast_shadows = true;
        bool _gi_only = false;
        std::string _path;
        Transform _transform;
        AABB _bounds;
    };
}
