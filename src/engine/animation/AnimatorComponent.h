//
// Created by alecpizz on 6/29/25.
//
#pragma once
#include "SkeletonPose.h"

namespace cologne
{
    class SkinnedModel;
}

namespace cologne
{
    class Skeleton;
    class AnimationClip;
    class AnimatorComponent
    {
    public:
        explicit AnimatorComponent(const SkinnedModel& model);
        explicit AnimatorComponent(const Skeleton& skeleton);
        explicit AnimatorComponent(const std::string& model_name);
        void update_animation(float dt);
        void play_animation(AnimationClip* clip);
        const std::vector<glm::mat4> get_skinning_matrices() const;
        float get_current_progress() const;
        AnimationClip* get_current_clip() const;
    private:
        const Skeleton& _skeleton;
        SkeletonPose _pose;
        AnimationClip* _current_clip = nullptr;
        float _current_time = 0.0f;
    };
}
