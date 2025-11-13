//
// Created by alecpizz on 11/12/25.
//
#pragma once
#include <engine/asset_manager/AssetHandle.h>
#include <engine/audio/AudioClip.h>

namespace cologne
{
    struct AudioSourceComponent
    {
        AssetHandle<AudioClip> clip;
        bool loop = false;
        bool play_on_awake = true;
        bool spatialized = true;
        float volume = 1.0f;
        float pitch = 1.0f;
        float min_distance = 1.0f;
        float max_distance = 100.0f;

        ma_sound* sound_instance = nullptr;
        bool is_playing = false;
        AudioSourceComponent() = default;
    };
}
