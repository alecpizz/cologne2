#pragma once
#include "Texture.h"
#include "Vertex.h"
#include "Mesh.h"

namespace goon
{

    class Mesh
    {
    public:
        Mesh(const Vertex *vertices, size_t num_vertices,
             const uint32_t *indices, size_t num_indices,
             const Texture *textures,
             size_t num_textures);

        ~Mesh();

        Vertex *get_vertices();

        uint32_t *get_indices();

        size_t get_num_indices() const;

        Texture *get_textures();

        void draw() const;

    private:
        std::vector<Vertex> _vertices;
        std::vector<uint32_t> _indices;
        std::vector<Texture> _textures;
        uint32_t _vbo = 0;
        uint32_t _ibo = 0;
        uint32_t _vao = 0;
    };
}
