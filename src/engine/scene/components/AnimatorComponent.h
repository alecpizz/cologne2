//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/asset_manager/AssetHandle.h>

namespace cologne
{
    struct AnimatorComponent
    {
        struct AnimationLayer
        {
            AssetHandle<AnimationClip> clip;
            float time = 0.0f;
            float weight = 1.0f;
            bool loop = true;
            bool is_finished = false;

            AssetHandle<AnimationClip> fade_to_clip;
            float fade_time = 0.0f;
            float fade_to_time = 0.0f;
            float fade_duration = 0.0f;
            bool next_loop = true;
        };

        std::vector<AnimationLayer> layers;

        AnimatorComponent();

        void play(const AssetHandle<AnimationClip> &clip, int layer_index = 0, bool loop = true);

        void crossfade_to(const AssetHandle<AnimationClip> &name, float duration, int layer_index = 0, bool loop = true);
    };
}
