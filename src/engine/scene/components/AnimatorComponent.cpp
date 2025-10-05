//
// Created by alecpizz on 10/5/25.
//
#include "AnimatorComponent.h"
namespace cologne
    {
    AnimatorComponent::AnimatorComponent()
    {
        layers.emplace_back();
    }

    void AnimatorComponent::play(const std::string &clip_name, int layer_index, bool loop)
    {
        if (layer_index >= layers.size())
        {
            layers.resize(layer_index + 1);
        }

        layers[layer_index].clip_name = clip_name;
        layers[layer_index].time = 0.0f;
        layers[layer_index].loop = loop;
        layers[layer_index].fade_duration = 0.0f;
        layers[layer_index].is_finished = false;
    }

    void AnimatorComponent::crossfade_to(const std::string &name, float duration, int layer_index, bool loop)
    {
        if (layer_index >= layers.size())
        {
            layers.resize(layer_index + 1);
        }

        auto& layer = layers[layer_index];
        if (layer.clip_name == name) return;

        layer.fade_to_clip_name = name;
        layer.fade_time = 0.0f;
        layer.fade_to_time = 0.0f;
        layer.fade_duration = duration;
        layer.next_loop = loop;
    }
}