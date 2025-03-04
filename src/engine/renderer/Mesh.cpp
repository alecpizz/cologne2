#include "Mesh.h"
#include "ElementBuffer.h"
#include "VertexBuffer.h"
#include "VertexAttribute.h"

namespace goon
{
    struct Mesh::Impl
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Texture> textures;
        std::shared_ptr<VertexAttribute> vertex_attribute = nullptr;
        std::shared_ptr<VertexBuffer> vertex_buffer = nullptr;
        std::shared_ptr<ElementBuffer> element_buffer = nullptr;

        void init()
        {
            vertex_attribute = std::make_shared<VertexAttribute>();
            vertex_buffer = std::make_shared<VertexBuffer>(vertices.data(), vertices.size());
            element_buffer = std::make_shared<ElementBuffer>(indices.data(), indices.size());
        }
    };

    Mesh::Mesh(const Vertex *vertices, const size_t num_vertices, const uint32_t *indices, const size_t num_indices,
               const Texture *textures,
               const size_t num_textures)
    {
        _impl = new Impl();
        for (size_t i = 0; i < num_vertices; i++)
        {
            _impl->vertices.push_back(vertices[i]);
        }
        for (size_t i = 0; i < num_indices; i++)
        {
            _impl->indices.push_back(indices[i]);
        }

        for (size_t i = 0; i < num_textures; i++)
        {
            _impl->textures.push_back(textures[i]);
        }

    }

    Mesh::~Mesh()
    {
        delete _impl;
    }

    Vertex *Mesh::get_vertices() const
    {
        return _impl->vertices.data();
    }

    uint32_t *Mesh::get_indices() const
    {
        return _impl->indices.data();
    }

    Texture *Mesh::get_textures() const
    {
        return _impl->textures.data();
    }

    VertexBuffer* Mesh::get_vertex_buffer() const
    {
        return _impl->vertex_buffer.get();
    }

    ElementBuffer* Mesh::get_element_buffer() const
    {
        return _impl->element_buffer.get();
    }

    VertexAttribute* Mesh::get_vertex_attribute() const
    {
        return _impl->vertex_attribute.get();
    }
}
