//
// Created by alecpizz on 6/29/25.
//

#include "AnimatorComponent.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/core/Input.h>
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

    void AnimatorComponent::create_ragdoll(Entity entity_id)
    {
        SkeletonPose pose(_skeleton);
        for (size_t i = 0; i < _skeleton.get_bone_count(); i++)
        {
            pose._local_transforms[i] = _skeleton.get_bones()[i].local_bind_transform;
        }
        pose.update_skinning_matrices(_skeleton);
        _ragdoll_id = Physics::create_ragdoll(entity_id, _bone_to_ragdoll_map, _skeleton, pose._global_transforms);
        Physics::make_ragdoll_kinematic(_ragdoll_id);
    }

    void AnimatorComponent::update(float dt, glm::mat4 transform)
    {
        if (_current_state == State::ANIMATING)
        {
            update_animation(dt);
            sync_ragdoll_to_animation(transform);
        }
        else
        {
            update_pose_from_ragdoll(transform);
        }
    }

    void AnimatorComponent::update_animation(float dt)
    {
        if (!_current_clip)
        {
            return;
        }

        _current_time += _current_clip->get_ticks_per_second() * dt;
        if (_is_playing_one_shot)
        {
            if (_current_time >= _current_clip->get_duration())
            {
                //done playing the animation
                _current_clip = _base_clip;
                _current_time = 0.0f;
            }
        }
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

    void AnimatorComponent::update_pose_from_ragdoll(const glm::mat4 &transform)
    {
        if (_ragdoll_id == -1)
        {
            return;
        }
        glm::mat4 inverse_entity_transform = glm::inverse(transform);

        for (int i = 0; i < _skeleton.get_bone_count(); i++)
        {
            const auto &bone = _skeleton.get_bones()[i];
            std::string node_name = bone.name;
            glm::mat4 node_transform = bone.local_bind_transform;
            unsigned int parent_idx = bone.parent_idx;
            glm::mat4 parent_transform = (parent_idx == -1) ? glm::mat4(1.0f) : _pose._global_transforms[parent_idx];
            glm::mat4 global_transform = parent_transform * node_transform;

            if (_bone_to_ragdoll_map.contains(node_name))
            {
                global_transform = inverse_entity_transform * Physics::get_rigidbody_transform(
                                       _bone_to_ragdoll_map[node_name]);
            }

            _pose._global_transforms[i] = global_transform;
        }
        _pose.update_skinning_matrices_no_rebuild(_skeleton);
    }

    void AnimatorComponent::sync_ragdoll_to_animation(const glm::mat4 &transform)
    {
        if (_ragdoll_id == -1)
        {
            return;
        }
        std::unordered_map<std::string, glm::mat4> ragdoll_transforms(_bone_to_ragdoll_map.size());
        for (auto &pair: _bone_to_ragdoll_map)
        {
            int bone_idx = _skeleton.find_bone_index(pair.first);
            ragdoll_transforms[pair.first] = transform * _pose._global_transforms[bone_idx];
        }
        Physics::sync_ragdoll(_ragdoll_id, ragdoll_transforms);
    }

    void AnimatorComponent::play_base_animation(AnimationClip *clip)
    {
        _current_clip = clip;
        _base_clip = clip;
        _current_time = 0.0f;
    }

    void AnimatorComponent::play_one_shot_animation(AnimationClip *clip)
    {
        _current_clip = clip;
        _is_playing_one_shot = true;
        _current_time = 0.0f;
    }

    void AnimatorComponent::to_ragdoll()
    {
        if (_current_state == State::RAGDOLLING)
        {
            return;
        }
        if (_ragdoll_id == -1)
        {
            return;
        }
        _current_state = State::RAGDOLLING;
        Physics::make_ragdoll_active(_ragdoll_id);
    }

    void AnimatorComponent::to_kinematic()
    {
        if (_current_state == State::ANIMATING)
        {
            return;
        }
        if (_ragdoll_id == -1)
        {
            return;
        }
        _current_state = State::ANIMATING;
        Physics::make_ragdoll_kinematic(_ragdoll_id);
    }

    int AnimatorComponent::get_current_key_frame_idx() const
    {
        return 0;
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

    void AnimatorComponent::take_ragdoll_hit(glm::vec3 point, glm::vec3 normal)
    {
        if (_ragdoll_id == -1)
        {
            return;
        }
        if (_current_state != State::RAGDOLLING)
        {
            return;
        }

        uint32_t closest_body = 0;
        float dist = std::numeric_limits<float>::max();
        for (auto pair : _bone_to_ragdoll_map)
        {
            auto transform = Physics::get_rigidbody_transform(pair.second);
            float distance = glm::distance(glm::vec3(transform[3]), point);
            if (distance < dist)
            {
                closest_body = pair.second;
                dist = distance;
            }
        }
        Physics::add_impulse_force_at_position(closest_body, point, -normal * 200.0f);
    }
}
