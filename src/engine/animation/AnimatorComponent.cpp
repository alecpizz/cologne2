//
// Created by alecpizz on 6/29/25.
//

#include "AnimatorComponent.h"

#include <engine/asset_manager/AssetManager.h>
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

    void AnimatorComponent::play_animation(AnimationClip *clip)
    {
        _current_clip = clip;
        _current_time = 0.0f;
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
}
