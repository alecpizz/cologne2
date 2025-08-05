//
// Created by alecpizz on 5/27/25.
//

#include "SkinnedMesh.h"

#include <engine/Types.h>

namespace cologne
{
    SkinnedMesh::SkinnedMesh(const SkinnedMeshData& mesh_data)
    {
        _name = mesh_data.name;
        _vertices = mesh_data.vertices;
        _indices = mesh_data.indices;
        _indices_count = static_cast<uint32_t>(_indices.size());

        _material_index = mesh_data.material_index;
        _aabb = AABB(glm::vec4(mesh_data.aabb_min, 1.0), glm::vec4(mesh_data.aabb_max, 1.0));
        _first_index = mesh_data.first_index;
        _base_vertex = mesh_data.base_vertex;
        _vertex_count = _vertices.size();
    }


    std::string SkinnedMesh::get_name() const
    {
        return _name;
    }

    AABB SkinnedMesh::get_aabb() const
    {
        return _aabb;
    }

    uint32_t SkinnedMesh::get_first_index() const
    {
        return _first_index;
    }

    uint32_t SkinnedMesh::get_base_vertex() const
    {
        return _base_vertex;
    }

    SkinnedMesh::~SkinnedMesh()
    {
    }

    uint32_t SkinnedMesh::get_material_index() const
    {
        return _material_index;
    }


    uint32_t SkinnedMesh::get_vertex_count() const
    {
        return _vertex_count;
    }

    uint32_t SkinnedMesh::get_indices_count() const
    {
        return _indices_count;
    }
}
