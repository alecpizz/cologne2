//
// Created by alecpizz on 3/29/2025.
//

#include "Audio.h"

#include <engine/util/DebugScope.h>

#include "AudioClip.h"
#include "miniaudio.h"

namespace cologne::Audio
{
    // std::unordered_map<std::string, Mix_Music*> musics;
    // std::unordered_map<std::string, Mix_Chunk*> sounds;
    static ma_engine engine;
    constexpr int MAX_ONE_SHOTS = 64;
    static std::vector<ma_sound> _one_shot_pool;
    static std::vector<bool> _one_shot_pool_in_use;

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

        _one_shot_pool.resize(MAX_ONE_SHOTS);
        _one_shot_pool_in_use.resize(MAX_ONE_SHOTS, false);
    }

    void cleanup()
    {
        for (int i = 0; i < MAX_ONE_SHOTS; i++)
        {
            if (_one_shot_pool_in_use[i])
            {
                ma_sound_uninit(&_one_shot_pool[i]);
            }
        }

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

    void update_one_shot_pool()
    {
        for (int i = 0; i < MAX_ONE_SHOTS; i++)
        {
            if (_one_shot_pool_in_use[i])
            {
                if (!ma_sound_is_playing(&_one_shot_pool[i]))
                {
                    ma_sound_uninit(&_one_shot_pool[i]);
                    _one_shot_pool_in_use[i] = false;
                }
            }
        }
    }

    ma_sound* play_audio_clip(const AssetHandle<AudioClip>& clip)
    {
        if (!clip)
        {
            return nullptr;
        }

        if (!clip->get_sound_prototype())
        {
            return nullptr;
        }

        int free_idx = -1;
        for (int i = 0; i < MAX_ONE_SHOTS; i++)
        {
            if (!_one_shot_pool_in_use[i])
            {
                free_idx = i;
                break;
            }
        }

        if (free_idx == -1)
        {
            LOG_WARN("No voices left!");
            return nullptr;
        }

        _one_shot_pool_in_use[free_idx] = true;
        ma_sound *sound = &_one_shot_pool[free_idx];
        ma_result result = ma_sound_init_copy(get_engine(), clip->get_sound_prototype(),
                                              MA_SOUND_FLAG_DECODE, NULL, sound);
        if (result != MA_SUCCESS)
        {
            LOG_WARN("play_one_shot: Failed to init_copy sound: %s", clip.handle.c_str());
            _one_shot_pool_in_use[free_idx] = false;
            return nullptr;
        }
        return sound;
    }

    void play_one_shot(AssetHandle<AudioClip> clip, glm::vec3 pos, float volume)
    {
        ma_sound* sound = play_audio_clip(clip);
        if (!sound)
        {
            return;
        }
        ma_sound_set_spatialization_enabled(sound, MA_TRUE);
        ma_sound_set_position(sound, pos.x, pos.y, pos.z);
        ma_sound_set_volume(sound, volume);
        ma_sound_set_looping(sound, MA_FALSE);
        ma_sound_start(sound);
    }

    void play_one_shot2D(AssetHandle<AudioClip> clip, float volume)
    {
        ma_sound* sound = play_audio_clip(clip);
        if (!sound)
        {
            return;
        }
        ma_sound_set_spatialization_enabled(sound, MA_FALSE);
        ma_sound_set_volume(sound, volume);
        ma_sound_set_looping(sound, MA_FALSE);
        ma_sound_start(sound);
    }
}
