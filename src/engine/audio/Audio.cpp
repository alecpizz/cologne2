//
// Created by alecpizz on 3/29/2025.
//

#include "Audio.h"

#include <engine/util/DebugScope.h>

#include "miniaudio.h"

namespace cologne::Audio
{
    // std::unordered_map<std::string, Mix_Music*> musics;
    // std::unordered_map<std::string, Mix_Chunk*> sounds;
    static ma_engine engine;
    void init()
    {
        DebugScope scope("Audio::init");
        ma_result result = ma_engine_init(nullptr, &engine);
        if (result != MA_SUCCESS)
        {
            LOG_ERROR("FUCK");
            return;
        }
        ma_engine_listener_set_position(&engine, 0, 0, 0, 0);
    }

    void cleanup()
    {
        ma_engine_uninit(&engine);
    }

    ma_engine *get_engine()
    {
        return &engine;
    }

    ma_result load_sound(const std::string &path, ma_sound *sound)
    {
        ma_result result = ma_sound_init_from_file(&engine, path.c_str(),
            MA_SOUND_FLAG_DECODE, nullptr, nullptr, sound);
        if (result != MA_SUCCESS)
        {
            LOG_WARN("Failed to load sound %s", path.c_str());
        }
        return result;
    }

    void set_listener_position(const glm::vec3 &pos, const glm::vec3 &fwd, const glm::vec3 &up)
    {
        ma_engine_listener_set_position(&engine, 0, pos.x, pos.y, pos.z);
        ma_engine_listener_set_direction(&engine, 0, fwd.x, fwd.y, fwd.z);
        ma_engine_listener_set_world_up(&engine, 0, up.x, up.y, up.z);
    }


}
