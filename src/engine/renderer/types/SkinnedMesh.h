//
#pragma once
#include <engine/AABB.h>

#include "Vertex.h"

namespace cologne
{
    struct SkinnedMeshData;

    class SkinnedMesh
    {
    public:
        SkinnedMesh(const SkinnedMeshData& mesh_data);
        SkinnedMesh(const SkinnedMesh &mesh) = default;
        ~SkinnedMesh();
        std::string get_name() const;
        AABB get_aabb() const;
        uint32_t get_first_index() const;
        uint32_t get_base_vertex() const;
        uint32_t get_material_index() const;
        void draw(int32_t count = 1) const;
        uint32_t get_vertex_count() const;
        uint32_t get_indices_count() const;

    private:
        std::vector<WeightedVertex> _vertices;
        std::vector<uint32_t> _indices;
        std::string _name;
        AABB _aabb;
        uint32_t _material_index = 0;
        uint32_t _indices_count = 0;
        uint32_t _vertex_count = 0;
        uint32_t _base_vertex = 0;
        uint32_t _first_index = 0;
        uint32_t _vbo = 0;
        uint32_t _ibo = 0;
        uint32_t _vao = 0;
    };
}
