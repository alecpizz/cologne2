//
// Created by alecpizz on 6/29/25.
//

#include "Skeleton.h"

namespace cologne
{
    Skeleton::Skeleton(const std::vector<Bone> &bones, const std::unordered_map<std::string, int> &bone_map)
    {
        _bones = bones;
        _bone_name_to_index = bone_map;
    }

    int Skeleton::find_bone_index(const std::string &name) const
    {
        if (const auto it = _bone_name_to_index.find(name); it != _bone_name_to_index.end())
        {
            return it->second;
        }
        return -1;
    }

    int Skeleton::try_find_bone_index(const std::string &content) const
    {
        for (int i = 0; i < _bones.size(); i++)
        {
            if (_bones[i].name.find(content) != std::string::npos)
            {
                return i;
            }
        }
        return -1;
    }

    const Bone *Skeleton::find_bone_with_content(const std::string &content) const
    {
        for (auto &bone: _bones)
        {
            if (bone.name.find(content) != std::string::npos)
            {
                return &bone;
            }
        }
        return nullptr;
    }

    const std::vector<Bone> &Skeleton::get_bones() const
    {
        return _bones;
    }

    size_t Skeleton::get_bone_count() const
    {
        return _bones.size();
    }
}
