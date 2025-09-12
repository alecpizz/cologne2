//
// Created by alecpizz on 8/13/25.
//

#include "AnimationSystem.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/scene/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void AnimationSystem::on_scene_start(Scene *scene)
    {
        auto& registry = scene->get_raw_registry();
        for (const auto entity : registry.view<SkinnedModelComponent>())
        {
            auto& sk = registry.get<SkinnedModelComponent>(entity);
            auto model = AssetManager::get_skinned_model_by_name(sk.model_name);
            if (!model)
            {
                continue;
            }
            sk.skeleton = model->get_skeleton();
            sk.skeleton_pose = SkeletonPose(sk.skeleton);
        }

        for (const auto entity : registry.view<AnimatorComponent>())
        {
            auto& anim = registry.get<AnimatorComponent>(entity);
            anim.current_clip_name = anim.base_clip_name;
        }

    }

    void AnimationSystem::on_update(Scene* scene, float dt)
    {
        auto& registry = scene->get_raw_registry();
        auto animators = registry.view<AnimatorComponent, ActiveComponent, SkinnedModelComponent>();
        for (auto entity : animators)
        {
            if (!registry.get<ActiveComponent>(entity).active)
            {
                continue;
            }

            auto& animator = registry.get<AnimatorComponent>(entity);
            auto& skinned_model = registry.get<SkinnedModelComponent>(entity);
            auto current_clip = AssetManager::get_animation_by_name(animator.current_clip_name);
            if (!current_clip)
            {
                continue;
            }
            animator.current_time += current_clip->get_ticks_per_second() * dt;
            if (!animator.one_shot_name.empty())
            {
                if (animator.current_time >= current_clip->get_duration())
                {
                    animator.current_clip_name = animator.base_clip_name;
                    animator.current_time = 0.0f;
                    animator.one_shot_name = "";
                }
            }
            animator.current_time = std::fmod(animator.current_time, current_clip->get_duration());
            const auto &bones = skinned_model.skeleton.get_bones();
            for (size_t i = 0; i < bones.size(); i++)
            {
                const auto& bone = bones[i];
                if (BoneAnimationData *channel = current_clip->find_bone_channel(bone.name))
                {
                    channel->update(animator.current_time);
                    skinned_model.skeleton_pose.local_transforms[i] = channel->get_transform();
                }
                else
                {
                    skinned_model.skeleton_pose.local_transforms[i] = bone.local_bind_transform;
                }
            }
            skinned_model.skeleton_pose.update_skinning_matrices(skinned_model.skeleton);
        }
    }
}
