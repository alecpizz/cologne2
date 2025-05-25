//
// Created by alecpizz on 5/25/2025.
//

#include "AnimatedModel.h"

namespace cologne
{
    AnimatedModel::AnimatedModel(const char *path)
    {
    }

    AnimatedModel::~AnimatedModel()
    {
    }

    Transform &AnimatedModel::get_transform()
    {
        return _transform;
    }

    AABB AnimatedModel::get_aabb() const
    {
        return _bounds;
    }

    Material *AnimatedModel::get_materials()
    {
        return _materials.data();
    }

    uint64_t AnimatedModel::get_num_materials() const
    {
        return _materials.size();
    }

    Mesh *AnimatedModel::get_meshes()
    {
        return _meshes.data();
    }

    uint64_t AnimatedModel::get_num_meshes() const
    {
        return _meshes.size();
    }

    void AnimatedModel::set_active(bool active)
    {
        _active = active;
    }

    void AnimatedModel::set_aabb(AABB aabb)
    {
        _bounds = aabb;
    }

    bool AnimatedModel::get_active() const
    {
        return _active;
    }
}
