#include "Mesh.h"


namespace cologne
{

    Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, uint32_t material)
    {
        _vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
        _indices.insert(_indices.end(), indices.begin(), indices.end());
        _indices_count = _indices.size();

        glCreateBuffers(1, &_vbo);
        glNamedBufferStorage(_vbo, sizeof(Vertex) * _vertices.size(), _vertices.data(), GL_MAP_READ_BIT);

        glCreateBuffers(1, &_ibo);
        glNamedBufferStorage(_ibo, sizeof(uint32_t) * _indices.size(), _indices.data(), GL_MAP_READ_BIT);

        glCreateVertexArrays(1, &_vao);

        glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(_vao, _ibo);

        glEnableVertexArrayAttrib(_vao, 0);
        glEnableVertexArrayAttrib(_vao, 1);
        glEnableVertexArrayAttrib(_vao, 2);
        glEnableVertexArrayAttrib(_vao, 3);

        glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribFormat(_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribFormat(_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
        glVertexArrayAttribFormat(_vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent));

        glVertexArrayAttribBinding(_vao, 0, 0);
        glVertexArrayAttribBinding(_vao, 1, 0);
        glVertexArrayAttribBinding(_vao, 2, 0);
        glVertexArrayAttribBinding(_vao, 3, 0);

        _material_index = material;

    }

    Mesh::~Mesh()
    {
        // glDeleteBuffers(1, &_vbo);
        // glDeleteBuffers(1, &_ibo);
        // glDeleteVertexArrays(1, &_vao);
    }

    uint32_t Mesh::get_material_index() const
    {
        return _material_index;
    }


    void Mesh::draw() const
    {
        glBindVertexArray(_vao);
        glDrawElements(GL_TRIANGLES, _indices_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    std::vector<Vertex> Mesh::get_vertices() const
    {
        return _vertices;
    }

    std::vector<uint32_t> Mesh::get_indices() const
    {
        return _indices;
    }
}
