//
// Created by alecpizz on 3/29/2025.
//

#pragma once
#include <engine/asset_manager/AssetHandle.h>

#include "miniaudio.h"

namespace cologne::Audio
{
    void init();
    void cleanup();
    ma_engine* get_engine();
    ma_result load_sound(const std::string& path, ma_sound* sound);
    void set_listener_position(const glm::vec3& pos, const glm::vec3& fwd, const glm::vec3& up);

    void update_one_shot_pool();
    void play_one_shot(AssetHandle<AudioClip> clip, glm::vec3 pos, float volume = 1.0f);
    void play_one_shot2D(AssetHandle<AudioClip> clip, float volume = 1.0f);
}
