//
// Created by alecpizz on 8/13/25.
//

#include "AnimationSystem.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/scene/Components/Components.h>
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
            auto& sm = registry.get<SkinnedModelComponent>(entity);

            if (animator.source_clip_name.empty())
            {
                if (!animator.dest_clip_name.empty())
                {
                    animator.source_clip_name = animator.dest_clip_name;
                    animator.dest_clip_name = "";
                    animator.dest_time = 0.0f;
                }
            }

            auto source_clip = AssetManager::get_animation_by_name(animator.source_clip_name);
            auto dest_clip = AssetManager::get_animation_by_name(animator.dest_clip_name);

            if (!source_clip)
            {
                continue;
            }

            bool is_over = animator.source_time + animator.source_time + source_clip->get_ticks_per_second() * dt >= source_clip->get_duration();
            animator.source_time = std::fmod(animator.source_time + source_clip->get_ticks_per_second() * dt, source_clip->get_duration());

            bool is_blending = dest_clip && (animator.blend_time < animator.blend_duration);
            if (is_blending)
            {
                animator.blend_time += dt;
                animator.dest_time = std::fmod(animator.dest_time + dest_clip->get_ticks_per_second() * dt, dest_clip->get_duration());
            }

            for (size_t i = 0; i < sm.skeleton.get_bone_count(); i++)
            {
                const auto& bone = sm.skeleton.get_bones()[i];
                glm::mat4 final_local_transform;

                glm::mat4 source_transform = bone.local_bind_transform;
                if (auto* channel = source_clip->find_bone_channel(bone.name))
                {
                    channel->update(animator.source_time);
                    source_transform = channel->get_transform();
                }

                if (is_blending)
                {
                    glm::mat4 dest_transform = bone.local_bind_transform;
                    if (auto* channel = dest_clip->find_bone_channel(bone.name))
                    {
                        channel->update(animator.dest_time);
                        dest_transform = channel->get_transform();
                    }

                    glm::vec3 source_pos, source_scale;
                    glm::quat source_rot;
                    Util::decompose_mat4(source_transform, source_pos, source_rot, source_scale);

                    glm::vec3 dest_pos, dest_scale;
                    glm::quat dest_rot;
                    Util::decompose_mat4(dest_transform, dest_pos, dest_rot, dest_scale);

                    float blend_factor = animator.blend_time / animator.blend_duration;

                    glm::vec3 final_pos = glm::mix(source_pos, dest_pos, blend_factor);
                    glm::quat final_rot = glm::slerp(source_rot, dest_rot, blend_factor);
                    glm::vec3 final_scale = glm::mix(source_scale, dest_scale, blend_factor);

                    final_local_transform = glm::translate(glm::mat4(1.0f), final_pos)
                    * glm::toMat4(final_rot) * glm::scale(glm::mat4(1.0f), final_scale);
                }
                else
                {
                    final_local_transform = source_transform;
                }

                sm.skeleton_pose.local_transforms[i] = final_local_transform;
            }

            if (is_blending && animator.blend_time >= animator.blend_duration)
            {
                animator.source_clip_name = animator.dest_clip_name;
                animator.source_time = animator.dest_time;
                animator.dest_clip_name = "";
                animator.blend_time = 0.0f;
            }

            if (!animator.one_shot_name.empty() && animator.source_clip_name == animator.one_shot_name && !is_blending)
            {
                if (is_over)
                {
                    LOG_INFO("OVER");
                    animator.one_shot_name = "";
                    animator.crossfade_to(animator.one_shot_return_clip);
                }
                else
                {
                    LOG_INFO("%f", animator.source_time);
                }
            }

            sm.skeleton_pose.update_skinning_matrices(sm.skeleton);
        }
    }
}
