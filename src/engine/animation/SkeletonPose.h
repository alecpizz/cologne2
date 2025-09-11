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

        SkeletonPose() = default;

        void update_skinning_matrices(const Skeleton& skeleton);
        void update_skinning_matrices_no_rebuild(const Skeleton& skeleton);
        const std::vector<glm::mat4> get_skinning_matrices() const;
    public:
        std::vector<glm::mat4> local_transforms;
        std::vector<glm::mat4> _global_transforms;
    private:
        std::vector<glm::mat4> _skinning_matrices;
    };
}
