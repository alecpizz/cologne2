//
// Created by alecpizz on 9/3/25.
//

#include "AnimationSystem2.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/scene/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void AnimationSystem2::on_update(Scene *scene, float dt)
    {
        auto& registry = scene->get_raw_registry();
        auto view = registry.view<AnimatorComponent2, SkeletonComponent, SkeletonPoseComponent>();

        for (auto entity : view)
        {
            auto& animator = view.get<AnimatorComponent2>(entity);
            auto& skeleton = view.get<SkeletonComponent>(entity);
            auto& skeleton_pose = view.get<SkeletonPoseComponent>(entity);

            std::string clip = animator.is_playing_one_shot ? animator.one_shot_animation_clip : animator.base_animation_clip;

            if (clip.empty())
            {
                continue;
            }

            AnimationClip* animation_clip = AssetManager::get_animation_by_name(clip);
            if (!animation_clip)
            {
                continue;
            }

            animator.current_time += animation_clip->get_ticks_per_second() * dt * animator.speed;
            if (animator.is_playing_one_shot && animator.current_time >= animation_clip->get_duration())
            {
                animator.is_playing_one_shot = false;
                animator.one_shot_animation_clip = "";
                animator.current_time = 0.0f;
                continue;
            }
            animator.current_time = std::fmod(animator.current_time, animation_clip->get_duration());

            const auto& bones = skeleton.skeleton.get_bones();
            for (size_t i = 0; i < bones.size(); i++)
            {
                const auto &bone = bones[i];
                if (BoneAnimationData *channel = animation_clip->find_bone_channel(bone.name))
                {
                    channel->update(animator.current_time);
                    skeleton_pose.local_transforms[i] = channel->get_transform();
                }
                else
                {
                    skeleton_pose.local_transforms[i] = bone.local_bind_transform;
                }
            }

            for (size_t i = 0; i < bones.size(); i++)
            {
                const auto& bone = bones[i];
                if (bone.parent_idx == -1)
                {
                    skeleton_pose.global_transforms[i] = skeleton_pose.local_transforms[i];
                }
                else
                {
                    skeleton_pose.global_transforms[i] = skeleton_pose.global_transforms[bone.parent_idx] * skeleton_pose.local_transforms[i];
                }
            }

            for (size_t i = 0; i < bones.size(); i++)
            {
                skeleton_pose.skinning_matrices[i] = skeleton_pose.global_transforms[i] * bones[i].inverse_bind_pose;
            }
        }
    }

}
