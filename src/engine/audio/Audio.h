//
// Created by alecpizz on 3/29/2025.
//

#pragma once
#include "miniaudio.h"

namespace cologne::Audio
{
    void init();
    void cleanup();
    ma_engine* get_engine();
    ma_result load_sound(const std::string& path, ma_sound* sound);
    void set_listener_position(const glm::vec3& pos, const glm::vec3& fwd, const glm::vec3& up);
}
