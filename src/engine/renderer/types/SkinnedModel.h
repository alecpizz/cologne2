//
// Created by alecpizz on 5/25/2025.
//

#pragma once
#include <engine/AABB.h>
#include <engine/Transform.h>
#include <engine/Types.h>

#include "Material.h"
#include "SkinnedMesh.h"

namespace cologne
{
    struct BoneInfo
    {
        int id;
        glm::mat4 offset;
    };

    class SkinnedModel
    {
        friend class Animation;
    public:
        explicit SkinnedModel(const SkinnedModelData& data);

        ~SkinnedModel();

        Transform &get_transform();

        AABB get_aabb() const;

        Material *get_materials();

        uint64_t get_num_materials() const;

        SkinnedMesh *get_meshes();

        uint64_t get_num_meshes() const;

        void set_active(bool active);

        void set_aabb(AABB aabb);

        std::string get_name() const;

        bool get_active() const;

        bool get_cast_shadows() const;
        void set_cast_shadows(bool b);

    private:
        std::unordered_map<std::string, BoneInfo> _bone_info_map;
        int _bone_count = 0;
        std::vector<Material> _materials;
        std::vector<SkinnedMesh> _meshes;
        bool _active = true;
        bool _cast_shadows = true;
        Transform _transform;
        AABB _bounds;
        std::string _name;
    };
}
