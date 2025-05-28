//
#pragma once
#include "Vertex.h"

namespace cologne
{
    class SkinnedMesh
    {
    public:
        SkinnedMesh(const std::vector<WeightedVertex>& vertices, const std::vector<uint32_t>& indices, uint32_t material_idx);
        ~SkinnedMesh();
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
