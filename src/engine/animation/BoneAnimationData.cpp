//
// Created by alecpizz on 5/27/25.
//

#include "BoneAnimationData.h"

#include "../util/Util.h"
#include "assimp/anim.h"

namespace cologne
{
    BoneAnimationData::BoneAnimationData(const std::string &name, const aiNodeAnim *channel)
        : _transform(1.0f), _name(name)
    {
        for (size_t i = 0; i < channel->mNumPositionKeys; i++)
        {
            const auto pos = channel->mPositionKeys[i].mValue;
            const float time = channel->mPositionKeys[i].mTime;
            _positions.emplace_back(AnimationKeyPosition(Util::ai_vec3_to_glm_vec3(pos), time));
        }

        for (size_t i = 0; i < channel->mNumRotationKeys; i++)
        {
            const auto rot = channel->mRotationKeys[i].mValue;
            const float time = channel->mRotationKeys[i].mTime;
            _rotations.emplace_back(AnimationKeyRotation(Util::ai_quat_to_glm_quat(rot), time));
        }

        for (size_t i = 0; i < channel->mNumScalingKeys; i++)
        {
            const auto scale = channel->mScalingKeys[i].mValue;
            const float time = channel->mScalingKeys[i].mTime;
            _scales.emplace_back(AnimationKeyScale(Util::ai_vec3_to_glm_vec3(scale), time));
        }
    }

    void BoneAnimationData::update(float animation_time)
    {
        if (_positions.size() == 1)
        {

        }
        if (_rotations.size() == 1)
        {

        }

        if (_scales.size() == 1)
        {

        }
        const glm::mat4 translation = interpolate_position(animation_time);
        const glm::mat4 rotation = interpolate_rotation(animation_time);
        const glm::mat4 scale = interpolate_scale(animation_time);
        _transform = translation * rotation * scale;
    }

    glm::mat4 BoneAnimationData::get_transform() const
    {
        return _transform;
    }

    std::string BoneAnimationData::get_name() const
    {
        return _name;
    }

    int BoneAnimationData::get_position_idx(float time)
    {
        for (int i = 0; i < _positions.size() - 1; i++)
        {
            if (time < _positions[i + 1].time_stamp)
            {
                return i;
            }
        }
        return -1;
    }

    int BoneAnimationData::get_rotation_idx(float time)
    {
        for (int i = 0; i < _rotations.size() - 1; i++)
        {
            if (time < _rotations[i + 1].time_stamp)
            {
                return i;
            }
        }
        return -1;
    }

    int BoneAnimationData::get_scale_idx(float time)
    {
        for (int i = 0; i < _scales.size() - 1; i++)
        {
            if (time < _scales[i + 1].time_stamp)
            {
                return i;
            }
        }
        return -1;
    }

    float BoneAnimationData::get_scale_factor(const float last_time_stamp, const float next_time_stamp,
                                              const float animation_time)
    {
        //literally just a lerp lmao
        const float half_way = animation_time - last_time_stamp;
        const float frame_delta = next_time_stamp - last_time_stamp;
        return half_way / frame_delta;
    }

    glm::mat4 BoneAnimationData::interpolate_position(float animation_time)
    {
        if (_positions.size() == 1)
        {
            return glm::translate(glm::mat4(1.0f), _positions[0].position);
        }

        int position_0 = get_position_idx(animation_time);
        int position_1 = position_0 + 1;
        float scale_factor = get_scale_factor(_positions[position_0].time_stamp,
                                              _positions[position_1].time_stamp, animation_time);
        glm::vec3 final_position = glm::mix(_positions[position_0].position, _positions[position_1].position,
                                            scale_factor);
        return glm::translate(glm::mat4(1.0f), final_position);
    }

    glm::mat4 BoneAnimationData::interpolate_rotation(float animation_time)
    {
        if (_rotations.size() == 1)
        {
            return glm::toMat4(_rotations[0].rotation);
        }

        int rotation_0 = get_rotation_idx(animation_time);
        int rotation_1 = rotation_0 + 1;
        float scale_factor = get_scale_factor(_rotations[rotation_0].time_stamp,
                                              _rotations[rotation_1].time_stamp, animation_time);
        glm::quat final_rotation = glm::slerp(_rotations[rotation_0].rotation, _rotations[rotation_1].rotation,
                                              scale_factor);
        return glm::toMat4(final_rotation);
    }

    glm::mat4 BoneAnimationData::interpolate_scale(float animation_time)
    {
        if (_scales.size() == 1)
        {
            return glm::scale(glm::mat4(1.0f), _scales[0].scale);
        }
        int scale_0 = get_scale_idx(animation_time);
        int scale_1 = scale_0 + 1;
        float scale_factor = get_scale_factor(_scales[scale_0].time_stamp,
            _scales[scale_1].time_stamp, animation_time);
        glm::vec3 final_scale = glm::mix(_scales[scale_0].scale, _scales[scale_1].scale, scale_factor);
        return glm::scale(glm::mat4(1.0f), final_scale);
    }
}
