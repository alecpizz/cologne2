//
// Created by alecpizz on 6/29/25.
//
#pragma once

namespace cologne
{
    class Skeleton;
    class SkeletonPose
    {
    public:
        explicit SkeletonPose(const Skeleton& skeleton);
        void update_skinning_matrices(const Skeleton& skeleton);
        const std::vector<glm::mat4> get_skinning_matrices() const;
    public:
        std::vector<glm::mat4> _local_transforms;
    private:
        std::vector<glm::mat4> _global_transforms;
        std::vector<glm::mat4> _skinning_matrices;
    };
}
