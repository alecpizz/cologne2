#pragma once
namespace goon
{
    class Vertex;
    class Texture;
    struct VertexAttribute;
    struct VertexBuffer;
    struct ElementBuffer;

    class Mesh
    {
    public:
        Mesh(const Vertex *vertices, size_t num_vertices,
            const uint32_t *indices, size_t num_indices,
            const Texture *textures,
             size_t num_textures);

        ~Mesh();

        Vertex *get_vertices() const;

        uint32_t *get_indices() const;

        Texture *get_textures() const;

        VertexBuffer *get_vertex_buffer() const;

        ElementBuffer *get_element_buffer() const;

        VertexAttribute *get_vertex_attribute() const;

        Mesh(Mesh &&) = delete;

        Mesh(const Mesh &) = delete;

        Mesh &operator=(Mesh &&) = delete;

        Mesh &operator=(const Mesh &) = delete;

    private:
        struct Impl;
        Impl *_impl;
    };
}
