//
// Created by alecpizz on 5/27/25.
//

#include "SkinnedMesh.h"

namespace cologne
{
    SkinnedMesh::SkinnedMesh(const std::vector<WeightedVertex> &vertices, const std::vector<uint32_t> &indices, uint32_t material_idx)
    {
        _indices_count = static_cast<uint32_t>(indices.size());
        glCreateBuffers(1, &_vbo);
        glNamedBufferStorage(_vbo, sizeof(WeightedVertex) * vertices.size(),
            vertices.data(), GL_MAP_READ_BIT);

        glCreateBuffers(1, &_ibo);
        glNamedBufferStorage(_ibo, sizeof(uint32_t) * _indices_count, indices.data(), GL_MAP_READ_BIT);

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

        _material_index = material_idx;
    }

    SkinnedMesh::~SkinnedMesh()
    {
        //todo
    }

    uint32_t SkinnedMesh::get_material_index() const
    {
        return _material_index;
    }

    void SkinnedMesh::draw() const
    {
        glBindVertexArray(_vao);
        glDrawElements(GL_TRIANGLES, _indices_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}
