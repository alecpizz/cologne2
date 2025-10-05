//
// Created by alecpizz on 8/13/25.
//

#include "AnimationSystem.h"

#include <engine/animation/SkeletonPose.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/scene/components/SkinnedModelComponent.h>
#include <engine/scene/Scene.h>
#include <engine/scene/components/ActiveComponent.h>
#include <engine/util/Util.h>

namespace cologne
{
    void AnimationSystem::on_scene_start(Scene *scene)
    {
        auto &registry = scene->get_raw_registry();
        for (const auto entity: registry.view<SkinnedModelComponent>())
        {
            auto &sk = registry.get<SkinnedModelComponent>(entity);
            auto model = sk.model.get();
            if (!model)
            {
                continue;
            }
            sk.skeleton = model->get_skeleton();
            sk.skeleton_pose = SkeletonPose(sk.skeleton);
        }
    }

    void AnimationSystem::on_update(Scene *scene, float dt)
    {
        auto &registry = scene->get_raw_registry();
        auto animators = registry.view<AnimatorComponent, ActiveComponent, SkinnedModelComponent>();
        for (auto entity: animators)
        {
            if (!registry.get<ActiveComponent>(entity).active)
            {
                continue;
            }

            auto &animator = registry.get<AnimatorComponent>(entity);
            auto &sm = registry.get<SkinnedModelComponent>(entity);

            if (animator.layers.empty())
            {
                continue;
            }

            apply_animation(animator.layers[0], sm.skeleton, sm.skeleton_pose, dt);

            for (size_t i = 1; i < animator.layers.size(); i++)
            {
                //additive blending here
                apply_animation(animator.layers[i], sm.skeleton, sm.skeleton_pose, dt, animator.layers[i].weight);
            }

            sm.skeleton_pose.update_skinning_matrices(sm.skeleton);
        }
    }

    void AnimationSystem::apply_animation(AnimatorComponent::AnimationLayer &layer, const Skeleton &skeleton,
                                          SkeletonPose &skeleton_pose, float dt, float blend_weight)
    {
        auto current_clip = AssetManager::get_animation_by_name(layer.clip_name);
        if (!current_clip)
        {
            return;
        }

        float current_speed = current_clip->get_ticks_per_second();
        if (layer.loop)
        {
            layer.time = std::fmod(layer.time + current_speed * dt, current_clip->get_duration());
        }
        else
        {
            layer.time = std::min(layer.time + current_speed * dt, current_clip->get_duration());
        }

        if (!layer.loop && layer.time >= current_clip->get_duration() - 0.01f)
        {
            layer.is_finished = true;
            return;
        }

        if (layer.fade_duration > 0.0f)
        {
            auto fade_to_clip = AssetManager::get_animation_by_name(layer.fade_to_clip_name);
            if (fade_to_clip)
            {
                layer.fade_time += dt;
                float blend_alpha = std::min(layer.fade_time / layer.fade_duration, 1.0f);

                float fade_to_speed = fade_to_clip->get_ticks_per_second();

                if (layer.next_loop)
                {
                    layer.fade_to_time = std::fmod(layer.fade_to_time + fade_to_speed * dt, fade_to_clip->get_duration());
                }
                else
                {
                    layer.fade_to_time = std::min(layer.fade_to_time + fade_to_speed * dt, fade_to_clip->get_duration());
                }

                for (size_t i = 0; i < skeleton.get_bone_count(); i++)
                {
                    glm::mat4 final_transform = get_blended_transform(skeleton.get_bones()[i],
                        current_clip, layer.time,
                        fade_to_clip, layer.fade_to_time,
                        blend_alpha);
                    skeleton_pose.local_transforms[i] = final_transform;
                }

                if (layer.fade_time >= layer.fade_duration)
                {
                    layer.clip_name = layer.fade_to_clip_name;
                    layer.time = layer.fade_to_time;
                    layer.loop = layer.next_loop;
                    layer.fade_duration = 0.0f;
                }
            }
        }
        else
        {
            for (size_t i = 0; i < skeleton.get_bone_count(); i++)
            {
                glm::mat4 current_transform = get_bone_transform(skeleton.get_bones()[i], current_clip, layer.time);
                if (blend_weight < 1.0f)
                {
                    skeleton_pose.local_transforms[i] = blend_transforms(skeleton_pose.local_transforms[i], current_transform, blend_weight);
                }
                else
                {
                    skeleton_pose.local_transforms[i] = current_transform;
                }
            }
        }
    }

    glm::mat4 AnimationSystem::get_bone_transform(const Bone &bone, AnimationClip *clip, float time)
    {
        if (auto *channel = clip->find_bone_channel(bone.name))
        {
            channel->update(time);
            return channel->get_transform();
        }
        return bone.local_bind_transform;
    }

    glm::mat4 AnimationSystem::get_blended_transform(const Bone &bone, AnimationClip *clip_a, float time_a,
                                                     AnimationClip *clip_b, float time_b, float alpha)
    {
        glm::mat4 a = get_bone_transform(bone, clip_a, time_a);
        glm::mat4 b = get_bone_transform(bone, clip_b, time_b);
        return blend_transforms(a, b, alpha);
    }

    glm::mat4 AnimationSystem::blend_transforms(const glm::mat4 &transform_a, const glm::mat4 &transform_b, float alpha)
    {
        glm::vec3 pos_a, scale_a;
        glm::quat rot_a;
        Util::decompose_mat4(transform_a, pos_a, rot_a, scale_a);

        glm::vec3 pos_b, scale_b;
        glm::quat rot_b;
        Util::decompose_mat4(transform_b, pos_b, rot_b, scale_b);

        glm::vec3 final_pos = glm::mix(pos_a, pos_b, alpha);
        glm::quat final_rot = glm::slerp(rot_a, rot_b, alpha);
        glm::vec3 final_scale = glm::mix(scale_a, scale_b, alpha);

        return glm::translate(glm::mat4(1.0f), final_pos) *
               glm::toMat4(final_rot)
               * glm::scale(glm::mat4(1.0f), final_scale);
    }
}
