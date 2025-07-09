//
// Created by alecpizz on 5/25/2025.
//

#pragma once
#include <engine/AABB.h>
#include <engine/Types.h>

#include "Material.h"
#include "SkinnedMesh.h"

namespace cologne
{
    class SkinnedModel
    {
    public:
        SkinnedModel() = default;
        SkinnedModel(const std::vector<int32_t>& meshes, const std::string& name, const glm::vec3& min, const glm::vec3& max, const Skeleton& skeleton);

        ~SkinnedModel();

        AABB get_aabb() const;

        Material *get_materials();

        uint64_t get_num_materials() const;

        std::vector<int32_t>& get_mesh_indices();

        void set_active(bool active);

        void set_aabb(AABB aabb);

        std::string get_name() const;

        bool get_active() const;

        bool get_cast_shadows() const;
        void set_cast_shadows(bool b);
        const Skeleton& get_skeleton() const;

    private:
        Skeleton _skeleton;
        std::vector<int32_t> _mesh_indices = std::vector<int32_t>();
        std::vector<Material> _materials;
        bool _active = true;
        bool _cast_shadows = true;
        AABB _bounds;
        std::string _name;
    };
}
