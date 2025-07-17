//
// Created by alecpizz on 6/29/25.
//
#pragma once
#include "BoneAnimationData.h"


struct aiScene;
struct aiAnimation;

namespace cologne
{
    class AnimationClip
    {
    public:
        AnimationClip() = default;
        AnimationClip(const std::string& base_name, const aiAnimation* animation, const aiScene* scene);
        AnimationClip(const std::string& name, const std::unordered_map<std::string, BoneAnimationData>& data, float duration, int ticks);
        BoneAnimationData* find_bone_channel(const std::string& name);
        float get_ticks_per_second() const;
        float get_duration() const;
        const std::string& get_name() const;
        const std::unordered_map<std::string, BoneAnimationData>& get_data() const;
    private:
        std::unordered_map<std::string, BoneAnimationData> _bone_channels;
        std::string _name;
        float _duration = 0.0f;
        int _ticks_per_second = 0;
    };
}
