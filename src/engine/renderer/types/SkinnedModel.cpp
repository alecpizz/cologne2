//
// Created by alecpizz on 5/25/2025.
//

#include "SkinnedModel.h"

namespace cologne
{
    SkinnedModel::SkinnedModel(const SkinnedModelData& data)
    {
        _bone_info_map = data.bone_map;
        _bone_count = data.bone_count;
        _materials = data.materials;
        _bounds = AABB(data.aabb_min, data.aabb_max);
        for (auto& mesh : data.meshes)
        {
            _meshes.emplace_back(mesh.vertices, mesh.indices, mesh.material_index);
        }
        _name = data.name;
    }

    SkinnedModel::~SkinnedModel()
    {

    }

    AABB SkinnedModel::get_aabb() const
    {
        return _bounds;
    }

    Material *SkinnedModel::get_materials()
    {
        return _materials.data();
    }

    uint64_t SkinnedModel::get_num_materials() const
    {
        return _materials.size();
    }

    std::vector<SkinnedMesh>& SkinnedModel::get_meshes()
    {
        return _meshes;
    }

    void SkinnedModel::set_active(bool active)
    {
        _active = active;
    }

    void SkinnedModel::set_aabb(AABB aabb)
    {
        _bounds = aabb;
    }

    std::string SkinnedModel::get_name() const
    {
        return _name;
    }

    bool SkinnedModel::get_active() const
    {
        return _active;
    }

    bool SkinnedModel::get_cast_shadows() const
    {
        return _cast_shadows;
    }

    void SkinnedModel::set_cast_shadows(bool b)
    {
        _cast_shadows = b;
    }
}
