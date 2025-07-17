//
// Created by alecpizz on 6/29/25.
//

#include "AnimationClip.h"
#include "assimp/anim.h"
#include "assimp/scene.h"
#include "BoneAnimationData.h"

namespace cologne
{
    AnimationClip::AnimationClip(const std::string &base_name, const aiAnimation *animation, const aiScene *scene)
    {
        _name = base_name + "_" + animation->mName.C_Str();
        _duration = static_cast<float>(animation->mDuration);
        _ticks_per_second = static_cast<int>(animation->mTicksPerSecond);

        int size = animation->mNumChannels;
        for (int i = 0; i < size; i++)
        {
            const auto channel = animation->mChannels[i];
            std::string bone_name = channel->mNodeName.C_Str();
            _bone_channels[bone_name] = {bone_name, channel};
        }
    }

    AnimationClip::AnimationClip(const std::string &name,
        const std::unordered_map<std::string, BoneAnimationData> &data, float duration, int ticks)
    {
        _name = name;
        _duration = duration;
        _ticks_per_second = ticks;
        _bone_channels = data;
    }

    BoneAnimationData *AnimationClip::find_bone_channel(const std::string &name)
    {
        if (_bone_channels.contains(name))
        {
            return &_bone_channels.at(name);
        }
        return nullptr;
    }

    float AnimationClip::get_ticks_per_second() const
    {
        return _ticks_per_second;
    }

    float AnimationClip::get_duration() const
    {
        return _duration;
    }

    const std::string &AnimationClip::get_name() const
    {
        return _name;
    }

    const std::unordered_map<std::string, BoneAnimationData> & AnimationClip::get_data() const
    {
        return _bone_channels;
    }
}
