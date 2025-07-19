#include "Mesh.h"

#include <engine/Types.h>


namespace cologne
{

    Mesh::Mesh(const MeshData& mesh_data)
    {
        _name = mesh_data.name;
        _vertices.insert(_vertices.end(), mesh_data.vertices.begin(), mesh_data.vertices.end());
        _indices.insert(_indices.end(), mesh_data.indices.begin(), mesh_data.indices.end());
        _indices_count = _indices.size();
        _material_index = mesh_data.material_index;
        _inverse_bind_transform = mesh_data.inverse_bind_pose;
        _aabb = AABB(glm::vec4(mesh_data.aabb_min, 1.0), glm::vec4(mesh_data.aabb_max, 1.0));
        _first_index = mesh_data.first_index;
        _base_vertex = mesh_data.base_vertex;
    }


    Mesh::~Mesh()
    {
        // glDeleteBuffers(1, &_vbo);
        // glDeleteBuffers(1, &_ibo);
        // glDeleteVertexArrays(1, &_vao);
    }

    uint32_t Mesh::get_material_index() const
    {
        return _material_index;
    }

    const std::vector<Vertex>& Mesh::get_vertices() const
    {
        return _vertices;
    }

    const std::vector<uint32_t>& Mesh::get_indices() const
    {
        return _indices;
    }

    uint32_t Mesh::get_indices_count() const
    {
        return _indices_count;
    }

    std::string Mesh::get_name() const
    {
        return _name;
    }

    AABB Mesh::get_aabb() const
    {
        return _aabb;
    }

    glm::mat4 Mesh::get_inverse_bind_pose() const
    {
        return _inverse_bind_transform;
    }

    uint32_t Mesh::get_first_index()
    {
        return _first_index;
    }

    uint32_t Mesh::get_base_vertex()
    {
        return _base_vertex;
    }
}
