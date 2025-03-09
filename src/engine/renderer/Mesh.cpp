#include "Mesh.h"
#include "ElementBuffer.h"
#include "VertexBuffer.h"
#include "VertexAttribute.h"
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
        _vertex_attribute = {};
        _vertex_buffer = VertexBuffer(vertices, num_vertices);
        _element_buffer = ElementBuffer(indices, num_indices);
        _vertex_attribute.release();
        _vertex_buffer.release();
        _element_buffer.release();
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

    VertexBuffer Mesh::get_vertex_buffer() const
    {
        return _vertex_buffer;
    }

    ElementBuffer Mesh::get_element_buffer() const
    {
        return _element_buffer;
    }

    VertexAttribute Mesh::get_vertex_attribute() const
    {
        return _vertex_attribute;
    }

    void Mesh::draw() const
    {
        glBindVertexArray(_vertex_attribute.handle);
        glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, nullptr);
    }
}
