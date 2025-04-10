//
// Created by alecpizz on 3/29/2025.
//

#pragma once

namespace cologne::Audio
{
    void init();
    void add_sound(const char* sound_path);
    void add_music(const char* music_path);
    void play_sound(const char* sound_path, int volume);
    void stop_sound(const char* sound_path);
    void play_music(const char* music_path);
    void stop_music(const char* music_path);
    void set_music_volume(int volume);
    void destroy();
}
