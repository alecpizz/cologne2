//
// Created by alecpizz on 11/12/25.
//
#pragma once
#include "Audio.h"
#include "miniaudio.h"

namespace cologne
{
    class AudioClip
    {
    public:
        AudioClip(const std::string &path, const std::string &name) : _path(path), _name(name), _is_loaded(false)
        {
        }

        ~AudioClip()
        {
            if (_is_loaded)
            {
                ma_sound_uninit(&_sound);
                _is_loaded = false;
            }
        }

        AudioClip(const AudioClip &) = delete;

        AudioClip &operator=(const AudioClip &) = delete;

        AudioClip(AudioClip &&other) noexcept : _path(std::move(other._path)),
                                                _name(std::move(other._name)),
                                                _sound(other._sound),
                                                _is_loaded(other._is_loaded)
        {
            other._is_loaded = false;
        }

        AudioClip &operator=(AudioClip &&other) noexcept
        {
            if (this != &other)
            {
                if (_is_loaded)
                {
                    ma_sound_uninit(&_sound);
                }
                _path = std::move(other._path);
                _name = std::move(other._name);
                _sound = other._sound;
                _is_loaded = other._is_loaded;
                other._is_loaded = false;
            }
            return *this;
        }

        void load()
        {
            if (_is_loaded)
            {
                return;
            }
            ma_result result = Audio::load_sound(_path, &_sound);
            if (result == MA_SUCCESS)
            {
                _is_loaded = true;
            }
            else
            {
                LOG_ERROR("Failed to load audio clip: %s", _path.c_str());
            }
        }

        const std::string &get_name() const { return _name; }

        ma_sound *get_sound_prototype()
        {
            return _is_loaded ? &_sound : nullptr;
        }

    private:
        std::string _path;
        std::string _name;
        ma_sound _sound;
        bool _is_loaded = false;
    };
}
