//
// Created by alecpizz on 6/29/25.
//

#include "AnimatorComponent.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/physics/Physics.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/SkinnedModel.h>

#include "AnimationClip.h"
#include "Skeleton.h"

namespace cologne
{
    AnimatorComponent::AnimatorComponent(const SkinnedModel &model) : _skeleton(model.get_skeleton()),
                                                                      _pose(model.get_skeleton())
    {
        _current_time = 0.0f;
        _current_clip = nullptr;
    }

    AnimatorComponent::AnimatorComponent(const Skeleton &skeleton) : _skeleton(skeleton), _pose(skeleton)
    {
        _current_time = 0.0f;
        _current_clip = nullptr;
    }

    AnimatorComponent::AnimatorComponent(const std::string &model_name) : _skeleton(
                                                                              AssetManager::get_skinned_model_by_name(
                                                                                  model_name)->get_skeleton()),
                                                                          _pose(AssetManager::get_skinned_model_by_name(
                                                                              model_name)->get_skeleton())
    {
        _current_time = 0.0f;
        _current_clip = nullptr;
    }

    void AnimatorComponent::create_ragdoll(const RagdollCreateInfo &info)
    {
        _ragdoll_id = Physics::create_ragdoll(_bone_to_ragdoll_map, info);
    }

    void AnimatorComponent::update(float dt)
    {
        if (_current_state == State::ANIMATING)
        {
            update_animation(dt);
        }
        else
        {
            //ragdoll shit
            update_pose_from_ragdoll();
        }
    }

    void AnimatorComponent::update_animation(float dt)
    {
        if (!_current_clip)
        {
            return;
        }

        _current_time += _current_clip->get_ticks_per_second() * dt;
        _current_time = std::fmod(_current_time, _current_clip->get_duration());

        const auto &bones = _skeleton.get_bones();
        for (size_t i = 0; i < bones.size(); i++)
        {
            const auto &bone = bones[i];
            if (BoneAnimationData *channel = _current_clip->find_bone_channel(bone.name))
            {
                channel->update(_current_time);
                _pose._local_transforms[i] = channel->get_transform();
            }
            else
            {
                _pose._local_transforms[i] = bone.local_bind_transform;
            }
        }
        _pose.update_skinning_matrices(_skeleton);
    }

    void AnimatorComponent::update_pose_from_ragdoll()
    {
        // std::vector<glm::mat4> world_transforms(_skeleton.get_bone_count());
        // for (size_t i = 0; i < _skeleton.get_bone_count(); i++)
        // {
        //     const Bone &current_bone = _skeleton.get_bones()[i];
        //
        //     glm::mat4 parent_world_transform = glm::mat4(1.0f);
        //     if (current_bone.parent_idx != -1)
        //     {
        //         parent_world_transform = world_transforms[current_bone.parent_idx];
        //     }
        //
        //     glm::mat4 current_local_transform;
        //     glm::mat4 current_world_transform;
        //
        //     if (_bone_to_ragdoll_map.contains(current_bone.name))
        //     {
        //         current_world_transform =
        //                 Physics::get_rigidbody_transform(_bone_to_ragdoll_map[current_bone.name]);
        //         Engine::get_renderer()->draw_sphere(current_world_transform[3], 0.02f, glm::vec3(1.0f, 0.0f, 0.0f));
        //         current_local_transform = glm::inverse(parent_world_transform) * current_world_transform;
        //     }
        //     else
        //     {
        //         current_local_transform = current_bone.local_bind_transform;
        //         current_world_transform = parent_world_transform * current_local_transform;
        //     }
        //     world_transforms[i] = current_world_transform;
        //     glm::vec3 bone_position = world_transforms[i][3];
        //     Engine::get_renderer()->draw_sphere(bone_position, 0.02f, glm::vec3(0.0f, 1.0f, 0.0f));
        //     _pose._local_transforms[i] = current_local_transform;
        // }
        // _pose.update_skinning_matrices(_skeleton);

        for (int i = 0; i < _skeleton.get_bone_count(); i++)
        {
            const auto& bone = _skeleton.get_bones()[i];
            std::string node_name = bone.name;
            glm::mat4 node_transform = glm::mat4(1);
            node_transform = bone.local_bind_transform;
            unsigned int parent_idx = bone.parent_idx;
            glm::mat4 parent_transform = (parent_idx == -1) ? glm::mat4(1) : _pose._global_transforms[parent_idx];
            glm::mat4 global_transform = parent_transform * node_transform;

            if (_bone_to_ragdoll_map.contains(node_name))
            {
                global_transform = Physics::get_rigidbody_transform(_bone_to_ragdoll_map[node_name]);
            }

            _pose._global_transforms[i] = global_transform;
        }
        _pose.update_skinning_matrices_no_rebuild(_skeleton);
    }

    void AnimatorComponent::play_animation(AnimationClip *clip)
    {
        _current_clip = clip;
        _current_time = 0.0f;
    }

    void AnimatorComponent::to_ragdoll()
    {
        if (_current_state == State::RAGDOLLING)
        {
            return;
        }
        _current_state = State::RAGDOLLING;
    }

    void AnimatorComponent::to_kinematic()
    {
        if (_current_state == State::ANIMATING)
        {
            return;
        }
        _current_state = State::ANIMATING;
    }

    const std::vector<glm::mat4> AnimatorComponent::get_skinning_matrices() const
    {
        return _pose.get_skinning_matrices();
    }

    float AnimatorComponent::get_current_progress() const
    {
        return _current_time;
    }

    AnimationClip *AnimatorComponent::get_current_clip() const
    {
        return _current_clip;
    }

    AnimatorComponent::State AnimatorComponent::get_current_state() const
    {
        return _current_state;
    }
}
