#pragma once
#include "Texture.h"
#include "Vertex.h"
#include "Mesh.h"

namespace cologne
{
    class Mesh
    {
    public:
        Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, uint32_t material);
        Mesh(const Mesh& mesh) = default;
        ~Mesh();

        uint32_t get_material_index() const;

        void draw() const;

        std::vector<Vertex> get_vertices() const;
        std::vector<uint32_t> get_indices() const;

    private:
        std::vector<Vertex> _vertices;
        std::vector<uint32_t> _indices;
        uint32_t _material_index = 0;
        uint32_t _indices_count = 0;
        uint32_t _vbo = 0;
        uint32_t _ibo = 0;
        uint32_t _vao = 0;
    };
}
