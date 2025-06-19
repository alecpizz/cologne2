//
// Created by alecpizz on 3/1/2025.
//

#include "Model.h"
#include <engine/physics/Physics.h>
#include "Mesh.h"


namespace cologne
{
    Model::Model(const ModelData& data)
    {
        for (auto& mesh : data.meshes)
        {
            _meshes.emplace_back(mesh);
        }
        _materials = data.materials;
        _bounds = AABB(data.aabb_min, data.aabb_max);
        _name = data.name;
    }

    Model::~Model()
    {
        _meshes.clear();
        LOG_INFO("YOU NEED TO CLEAN UP YOUR MESHES");
        _materials.clear();
    }


    AABB Model::get_aabb() const
    {
        return _bounds;
    }

    const char *Model::get_name() const
    {
        return _name.c_str();
    }

    Material *Model::get_materials()
    {
        return _materials.data();
    }


    std::vector<Mesh>& Model::get_meshes()
    {
        return _meshes;
    }

    void Model::set_active(bool active)
    {
        _active = active;
    }

    void Model::set_aabb(AABB aabb)
    {
        _bounds = aabb;
    }


    bool Model::get_gi_only() const
    {
        return _gi_only;
    }

    bool Model::get_cast_shadows() const
    {
        return _cast_shadows;
    }

    void Model::set_cast_shadows(bool b)
    {
        _cast_shadows = b;
    }

    void Model::set_gi_only(bool b)
    {
        _gi_only = b;
    }
}
