//
// Created by alecpizz on 10/5/25.
//
#pragma once

namespace cologne
{
    struct AnimatorComponent
    {
        struct AnimationLayer
        {
            std::string clip_name;
            float time = 0.0f;
            float weight = 1.0f;
            bool loop = true;
            bool is_finished = false;

            std::string fade_to_clip_name;
            float fade_time = 0.0f;
            float fade_to_time = 0.0f;
            float fade_duration = 0.0f;
            bool next_loop = true;
        };

        std::vector<AnimationLayer> layers;

        AnimatorComponent();

        void play(const std::string &clip_name, int layer_index = 0, bool loop = true);

        void crossfade_to(const std::string &name, float duration, int layer_index = 0, bool loop = true);
    };
}
