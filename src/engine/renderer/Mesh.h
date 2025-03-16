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
             const uint32_t *indices, size_t num_indices, uint32_t material);

        ~Mesh();
        uint32_t get_material_index() const;
        void draw() const;

    private:
        uint32_t _material_index = 0;
        uint32_t _indices_count = 0;
        uint32_t _vbo = 0;
        uint32_t _ibo = 0;
        uint32_t _vao = 0;
    };
}
