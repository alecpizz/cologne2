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
        glCreateBuffers(1, &_vbo);
        glNamedBufferStorage(_vbo, sizeof(WeightedVertex) * _vertices.size(),
            _vertices.data(), GL_MAP_READ_BIT);

        glCreateBuffers(1, &_ibo);
        glNamedBufferStorage(_ibo, sizeof(uint32_t) * _indices_count, _indices.data(), GL_MAP_READ_BIT);

        glCreateVertexArrays(1, &_vao);

        glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(WeightedVertex));
        glVertexArrayElementBuffer(_vao, _ibo);

        glEnableVertexArrayAttrib(_vao, 0);
        glEnableVertexArrayAttrib(_vao, 1);
        glEnableVertexArrayAttrib(_vao, 2);
        glEnableVertexArrayAttrib(_vao, 3);
        glEnableVertexArrayAttrib(_vao, 4);
        glEnableVertexArrayAttrib(_vao, 5);

        glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(WeightedVertex, position));
        glVertexArrayAttribFormat(_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(WeightedVertex, normal));
        glVertexArrayAttribFormat(_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(WeightedVertex, uv));
        glVertexArrayAttribFormat(_vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof(WeightedVertex, tangent));
        glVertexArrayAttribIFormat(_vao, 4, 4, GL_INT, offsetof(WeightedVertex, boneID));
        glVertexArrayAttribFormat(_vao, 5, 4, GL_FLOAT, GL_FALSE, offsetof(WeightedVertex, weight));

        glVertexArrayAttribBinding(_vao, 0, 0);
        glVertexArrayAttribBinding(_vao, 1, 0);
        glVertexArrayAttribBinding(_vao, 2, 0);
        glVertexArrayAttribBinding(_vao, 3, 0);
        glVertexArrayAttribBinding(_vao, 4, 0);
        glVertexArrayAttribBinding(_vao, 5, 0);

        _material_index = mesh_data.material_index;
        _aabb = AABB(mesh_data.aabb_min, mesh_data.aabb_max);
        _first_index = mesh_data.first_index;
        _base_vertex = mesh_data.base_vertex;
    }

    std::vector<WeightedVertex> SkinnedMesh::get_vertices() const
    {
        return _vertices;
    }

    std::vector<uint32_t> SkinnedMesh::get_indices() const
    {
        return _indices;
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
        //todo
    }

    uint32_t SkinnedMesh::get_material_index() const
    {
        return _material_index;
    }

    void SkinnedMesh::draw(int32_t count) const
    {
        if (count == 0)
        {
            return;
        }
        glBindVertexArray(_vao);
        glDrawElementsInstanced(GL_TRIANGLES, _indices_count, GL_UNSIGNED_INT, 0, count);
        glBindVertexArray(0);
    }
}
