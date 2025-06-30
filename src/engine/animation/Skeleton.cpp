//
// Created by alecpizz on 6/29/25.
//

#include "Skeleton.h"

namespace cologne
{
    int Skeleton::find_bone_index(const std::string &name) const
    {
        if (const auto it = _bone_name_to_index.find(name); it != _bone_name_to_index.end())
        {
            return it->second;
        }
        return -1;
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
