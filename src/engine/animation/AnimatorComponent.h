//
// Created by alecpizz on 6/29/25.
//
#pragma once
#include "Skeleton.h"
#include "SkeletonPose.h"

namespace cologne
{
    class Entity;
}

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

        AnimatorComponent() = default;
        AnimatorComponent(const AnimatorComponent &other);

        AnimatorComponent(AnimatorComponent &&other) noexcept;

        AnimatorComponent & operator=(const AnimatorComponent &other);

        AnimatorComponent & operator=(AnimatorComponent &&other) noexcept;

        explicit AnimatorComponent(const SkinnedModel& model);
        explicit AnimatorComponent(const std::string& model_name);
        void create_ragdoll(Entity entity_id);
        void update(float dt, glm::mat4 transform);
        void play_base_animation(AnimationClip* clip);
        void play_one_shot_animation(AnimationClip* clip);
        void to_ragdoll();
        void to_kinematic();
        int get_current_key_frame_idx() const;
        const std::vector<glm::mat4> get_skinning_matrices() const;
        float get_current_progress() const;
        AnimationClip* get_current_clip() const;
        AnimationClip* get_base_clip() const;
        void set_current_progress(float progress);
        State get_current_state() const;
        const std::string& get_model_base_name() const;
        int32_t get_ragdoll_id() const;
        void take_ragdoll_hit(glm::vec3 point, glm::vec3 normal);
        void set_has_ragdoll(bool b) {_has_ragdoll = b;}
        bool has_ragdoll() {return _has_ragdoll;}
    private:
        bool _has_ragdoll = false;
        State _current_state = State::ANIMATING;
        void update_animation(float dt);
        void update_pose_from_ragdoll(const glm::mat4& transform);
        void sync_ragdoll_to_animation(const glm::mat4& transform);
        int32_t _ragdoll_id = -1;
        std::unordered_map<std::string, uint32_t> _bone_to_ragdoll_map = std::unordered_map<std::string, uint32_t>();
        Skeleton _skeleton;
        std::string _model_name;
        SkeletonPose _pose;
        AnimationClip* _current_clip = nullptr;
        AnimationClip* _base_clip = nullptr;
        float _current_time = 0.0f;
        bool _is_playing_one_shot = false;
    };
}
