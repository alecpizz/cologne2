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
        _local_transforms.assign(bone_count, glm::mat4(1.0f));
        _global_transforms.assign(bone_count, glm::mat4(1.0f));
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
                _global_transforms[i] = _local_transforms[i];
            }
            else
            {
                _global_transforms[i] = _global_transforms[bone.parent_idx] * _local_transforms[i];
            }
        }

        for (size_t i = 0; i < bones.size(); i++)
        {
            _skinning_matrices[i] = _global_transforms[i] * bones[i].inverse_bind_pose;
        }
    }

    void SkeletonPose::update_skinning_matrices_no_rebuild(const Skeleton &skeleton)
    {
        const auto& bones = skeleton.get_bones();
        for (size_t i = 0; i < bones.size(); i++)
        {
            glm::vec3 scale = glm::vec3(_global_transforms[i][0][0], _global_transforms[i][1][1], _global_transforms[i][2][2]);
            glm::vec3 scale2 = glm::vec3(bones[i].inverse_bind_pose[0][0], bones[i].inverse_bind_pose[1][1], bones[i].inverse_bind_pose[2][2]);
            if (scale.x > 1.1f)
            {
                LOG_INFO("Big bone %s", bones[i].name.c_str());
            }
            _skinning_matrices[i] = _global_transforms[i] * bones[i].inverse_bind_pose;
        }
    }

    const std::vector<glm::mat4> SkeletonPose::get_skinning_matrices() const
    {
        return _skinning_matrices;
    }
}
