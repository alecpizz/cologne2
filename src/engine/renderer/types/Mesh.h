#pragma once

#include "Vertex.h"
#include "Mesh.h"

namespace cologne
{
    struct MeshData;

    class Mesh
    {
    public:
        Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, uint32_t material, const std::string& name);
        Mesh(const MeshData& mesh_data);
        Mesh(const Mesh& mesh) = default;
        ~Mesh();

        uint32_t get_material_index() const;

        void draw(int32_t count = 1) const;

        std::vector<Vertex> get_vertices() const;
        std::vector<uint32_t> get_indices() const;
        std::string get_name() const;
        glm::mat4 get_inverse_bind_pose() const;

    private:
        std::vector<Vertex> _vertices;
        std::vector<uint32_t> _indices;
        std::string _name;
        uint32_t _material_index = 0;
        uint32_t _indices_count = 0;
        uint32_t _vbo = 0;
        uint32_t _ibo = 0;
        uint32_t _vao = 0;
        glm::mat4 _inverse_bind_transform = glm::mat4(1.0f);
    };
}
