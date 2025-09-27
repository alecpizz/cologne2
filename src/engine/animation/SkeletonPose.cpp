//
// Created by alecpizz on 6/29/25.
//

#include "SkeletonPose.h"

#include "Skeleton.h"

namespace cologne
{
    SkeletonPose::SkeletonPose(const Skeleton &skeleton)
    {
        const auto bone_count = skeleton.get_bone_count();
        local_transforms.assign(bone_count, glm::mat4(1.0f));
        global_transforms.assign(bone_count, glm::mat4(1.0f));
        _skinning_matrices.assign(bone_count, glm::mat4(1.0f));
    }

    void SkeletonPose::update_skinning_matrices(const Skeleton &skeleton)
    {
        const auto& bones = skeleton.get_bones();
        for (size_t i = 0; i < bones.size(); i++)
        {
            const auto& bone = bones[i];
            if (bone.parent_idx == -1)
            {
                global_transforms[i] = local_transforms[i];
            }
            else
            {
                global_transforms[i] = global_transforms[bone.parent_idx] * local_transforms[i];
            }
        }

        for (size_t i = 0; i < bones.size(); i++)
        {
            _skinning_matrices[i] = global_transforms[i] * bones[i].inverse_bind_pose;
        }
    }

    void SkeletonPose::update_skinning_matrices_no_rebuild(const Skeleton &skeleton)
    {
        const auto& bones = skeleton.get_bones();
        for (size_t i = 0; i < bones.size(); i++)
        {
            _skinning_matrices[i] = global_transforms[i] * bones[i].inverse_bind_pose;
        }
    }

    const std::vector<glm::mat4> SkeletonPose::get_skinning_matrices() const
    {
        return _skinning_matrices;
    }
}
