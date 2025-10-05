//
// Created by alecpizz on 8/13/25.
//
#pragma once
#include <engine/scene/Components/Components.h>
#include <engine/scene/systems/System.h>

namespace cologne
{
    class AnimationClip;
}

namespace cologne
{
    class AnimationSystem : public System
    {
    public:
        void on_scene_start(Scene *scene) override;
        void on_update(Scene* scene, float dt) override;
        UpdateFlags get_update_flags() override
        {
            return RUNTIME;
        }

    private:
        void apply_animation(AnimatorComponent::AnimationLayer &layer, const Skeleton &skeleton,
                             SkeletonPose &skeleton_pose, float dt, float blend_weight = 1.0f);
        glm::mat4 get_bone_transform(const Bone& bone, AnimationClip* clip, float time);
        glm::mat4 get_blended_transform(const Bone& bone, AnimationClip* clip_a, float time_a, AnimationClip* clip_b,
            float time_b, float alpha);
        glm::mat4 blend_transforms(const glm::mat4& transform_a, const glm::mat4& transform_b, float alpha);
    };
}
