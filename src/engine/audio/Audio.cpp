//
// Created by alecpizz on 3/29/2025.
//

#include "Audio.h"

#include <engine/util/DebugScope.h>

namespace cologne::Audio
{
    // std::unordered_map<std::string, Mix_Music*> musics;
    // std::unordered_map<std::string, Mix_Chunk*> sounds;
    void init()
    {
        LOG_INFO("AUDIO INTI");
        DebugScope scope("Audio::init");
        LOG_ERROR("Audio not implemented");
    }

    void add_sound(const char *sound_path)
    {
        LOG_ERROR("Audio not implemented");
        // if (sounds.contains(sound_path))
        // {
        //     return;
        // }
        // auto s = Mix_LoadWAV(sound_path);
        // if (s == nullptr)
        // {
        //     LOG_ERROR("Failed to load sound file: %s", SDL_GetError());
        // }
        // sounds.insert(std::pair<std::string, Mix_Chunk*>(sound_path, s));
    }

    void add_music(const char *music_path)
    {
        LOG_ERROR("Audio not implemented");
        // auto m = Mix_LoadMUS(music_path);
        // if (m == nullptr)
        // {
        //     LOG_ERROR("Failed to load music file: %s", SDL_GetError());
        // }
        // musics.insert(std::pair<std::string, Mix_Music*>(music_path, m));
    }

    void play_sound(const char *sound_path, int volume)
    {
        // if (!sounds.contains(sound_path))
        // {
        //     LOG_WARN("No sound found for: %s, attempting to add it", sound_path);
        //     add_sound(sound_path);
        // }
        // auto channel = Mix_PlayChannel(-1, sounds[sound_path], 0);
        // Mix_Volume(channel, volume);
        LOG_ERROR("Audio not implemented");
    }

    void stop_sound(const char *sound_path)
    {
        LOG_ERROR("Audio not implemented");
    }

    void play_music(const char *music_path)
    {
        // if (!musics.contains(music_path))
        // {
        //     LOG_ERROR("No music found for: %s", music_path);
        //     return;
        // }
        // Mix_PlayMusic(musics[music_path], 9999);
        LOG_ERROR("Audio not implemented");
    }

    void stop_music(const char *music_path)
    {
       // Mix_HaltMusic();
        LOG_ERROR("Audio not implemented");
    }

    void set_music_volume(int volume)
    {
        // Mix_VolumeMusic(volume);
        LOG_ERROR("Audio not implemented");
    }

    void destroy()
    {
        // for (const auto& music : musics)
        // {
        //     Mix_FreeMusic(music.second);
        // }
        // for (const auto& sound : sounds)
        // {
        //     Mix_FreeChunk(sound.second);
        // }
        // Mix_CloseAudio();
        // Mix_Quit();
        LOG_ERROR("Audio not implemented");
    }
}
