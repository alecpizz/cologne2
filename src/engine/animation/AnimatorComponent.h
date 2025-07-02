//
// Created by alecpizz on 6/29/25.
//
#pragma once
#include "Skeleton.h"
#include "SkeletonPose.h"

namespace cologne
{
    struct RagdollCreateInfo;
}

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
        enum class State
        {
            ANIMATING,
            RAGDOLLING
        };
        explicit AnimatorComponent(const SkinnedModel& model);
        explicit AnimatorComponent(const Skeleton& skeleton);
        explicit AnimatorComponent(const std::string& model_name);
        AnimatorComponent(const AnimatorComponent&);
        AnimatorComponent(AnimatorComponent&&);
        void create_ragdoll();
        void update(float dt, glm::mat4 transform);
        void play_animation(AnimationClip* clip);
        void to_ragdoll();
        void to_kinematic();
        const std::vector<glm::mat4> get_skinning_matrices() const;
        float get_current_progress() const;
        AnimationClip* get_current_clip() const;
        State get_current_state() const;
    private:
        State _current_state = State::ANIMATING;
        void update_animation(float dt);
        void update_pose_from_ragdoll(const glm::mat4& transform);
        void sync_ragdoll_to_animation(const glm::mat4& transform);
        int32_t _ragdoll_id = -1;
        std::unordered_map<std::string, uint32_t> _bone_to_ragdoll_map = std::unordered_map<std::string, uint32_t>();
        const Skeleton& _skeleton;
        SkeletonPose _pose;
        AnimationClip* _current_clip = nullptr;
        float _current_time = 0.0f;
    };
}
