//
// Created by alecpizz on 3/1/2025.
//

#include "Model.h"
#include <engine/physics/Physics.h>
#include "Mesh.h"


namespace cologne
{
    Model::Model(const std::vector<int32_t>& meshes, const std::string& name, const glm::vec3& min, const glm::vec3& max)
    {
        _mesh_indices = meshes;
        _name = name;
        _bounds = AABB(glm::vec4(min, 1.0), glm::vec4(max, 1.0));
    }

    Model::~Model()
    {

    }


    AABB Model::get_aabb() const
    {
        return _bounds;
    }

    const char *Model::get_name() const
    {
        return _name.c_str();
    }


    std::vector<int32_t> & Model::get_mesh_indices()
    {
        return _mesh_indices;
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
