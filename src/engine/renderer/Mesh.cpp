#include "Mesh.h"
#include "Texture.h"

namespace goon
{
    Mesh::Mesh(const Vertex *vertices, const size_t num_vertices, const uint32_t *indices, const size_t num_indices,
               const Texture *textures,
               const size_t num_textures)
    {
        for (size_t i = 0; i < num_vertices; i++)
        {
            _vertices.emplace_back(vertices[i]);
        }
        for (size_t i = 0; i < num_indices; i++)
        {
            _indices.emplace_back(indices[i]);
        }

        for (size_t i = 0; i < num_textures; i++)
        {
            _textures.emplace_back(textures[i]);
        }
        glCreateBuffers(1, &_vbo);
        glNamedBufferStorage(_vbo, sizeof(Vertex)* num_vertices, vertices, GL_MAP_READ_BIT);

        glCreateBuffers(1, &_ibo);
        glNamedBufferStorage(_ibo, sizeof(uint32_t)*num_indices, indices, GL_MAP_READ_BIT);

        glCreateVertexArrays(1, &_vao);

        glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(_vao, _ibo);

        glEnableVertexArrayAttrib(_vao, 0);
        glEnableVertexArrayAttrib(_vao, 1);
        glEnableVertexArrayAttrib(_vao, 2);

        glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribFormat(_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribFormat(_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));

        glVertexArrayAttribBinding(_vao, 0, 0);
        glVertexArrayAttribBinding(_vao, 1, 0);
        glVertexArrayAttribBinding(_vao, 2, 0);
    }

    Mesh::~Mesh()
    {
    }

    Vertex *Mesh::get_vertices()
    {
        return _vertices.data();
    }

    uint32_t *Mesh::get_indices()
    {
        return _indices.data();
    }

    size_t Mesh::get_num_indices() const
    {
        return _indices.size();
    }

    Texture *Mesh::get_textures()
    {
        return _textures.data();
    }


    void Mesh::draw() const
    {
        glBindVertexArray(_vao);
        glDrawElements(GL_TRIANGLES, static_cast<uint32_t>(_indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}
