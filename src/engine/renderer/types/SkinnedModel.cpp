//
// Created by alecpizz on 5/25/2025.
//

#include "SkinnedModel.h"

namespace cologne
{
    SkinnedModel::SkinnedModel(const std::vector<int32_t> &meshes, const std::string &name, const glm::vec3 &min,
                               const glm::vec3 &max, const Skeleton &skeleton)
    {
        _skeleton = skeleton;
        _bounds = AABB(min, max);
        _name = name;
        _mesh_indices = meshes;
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

    std::vector<int32_t> &SkinnedModel::get_mesh_indices()
    {
        return _mesh_indices;
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

    const Skeleton &SkinnedModel::get_skeleton() const
    {
        return _skeleton;
    }
}
