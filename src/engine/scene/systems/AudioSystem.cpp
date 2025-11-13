//
// Created by alecpizz on 11/12/25.
//

#include "AudioSystem.h"

#include <engine/scene/Entity.h>
#include <engine/scene/Scene.h>
#include <engine/scene/components/AudioSourceComponent.h>
#include <engine/scene/components/TransformComponent.h>

namespace cologne
{
    void AudioSystem::on_create()
    {
    }

    void AudioSystem::on_scene_start(Scene *scene)
    {
        auto view = scene->get_raw_registry().view<AudioSourceComponent, TransformComponent>();
        for (auto entity: view)
        {
            auto &audio_src = view.get<AudioSourceComponent>(entity);
            auto &transform = view.get<TransformComponent>(entity);

            if (audio_src.sound_instance)
            {
                ma_sound_uninit(audio_src.sound_instance);
                delete audio_src.sound_instance;
                audio_src.sound_instance = nullptr;
            }

            auto *clip_asset = audio_src.clip.get();
            if (!clip_asset || !clip_asset->get_sound_prototype())
            {
                LOG_WARN("Audio source had invalid or unloaded clip %s", audio_src.clip.handle.c_str());
                continue;
            }

            audio_src.sound_instance = new ma_sound;
            ma_result result = ma_sound_init_copy(Audio::get_engine(),
                                                  clip_asset->get_sound_prototype(), MA_SOUND_FLAG_DECODE,
                                                  nullptr, audio_src.sound_instance);
            if (result != MA_SUCCESS)
            {
                LOG_WARN("Failed to init sound copy for %s", audio_src.clip.handle.c_str());
                delete audio_src.sound_instance;
                audio_src.sound_instance = nullptr;
                continue;
            }

            ma_sound_set_looping(audio_src.sound_instance, audio_src.loop);
            ma_sound_set_looping(audio_src.sound_instance, audio_src.loop);
            ma_sound_set_volume(audio_src.sound_instance, audio_src.volume);
            ma_sound_set_pitch(audio_src.sound_instance, audio_src.pitch);
            ma_sound_set_spatialization_enabled(audio_src.sound_instance, audio_src.spatialized);
            ma_sound_set_min_distance(audio_src.sound_instance, audio_src.min_distance);
            ma_sound_set_max_distance(audio_src.sound_instance, audio_src.max_distance);
            ma_sound_set_rolloff(audio_src.sound_instance, ma_attenuation_model_inverse);

            if (audio_src.spatialized)
            {
                ma_sound_set_position(audio_src.sound_instance,
                                      transform.position.x, transform.position.y, transform.position.z);
            }
            if (audio_src.play_on_awake)
            {
                ma_sound_start(audio_src.sound_instance);
                audio_src.is_playing = true;
            }
            else
            {
                audio_src.is_playing = false;
            }
        }
    }

    void AudioSystem::on_scene_exit(Scene *scene)
    {
        auto view = scene->get_raw_registry().view<AudioSourceComponent>();
        for (auto entity: view)
        {
            auto &audio_src = view.get<AudioSourceComponent>(entity);
            if (audio_src.sound_instance)
            {
                ma_sound_stop(audio_src.sound_instance);
                ma_sound_uninit(audio_src.sound_instance);
                delete audio_src.sound_instance;
                audio_src.sound_instance = nullptr;
                audio_src.is_playing = false;
            }
        }
    }

    void AudioSystem::on_update(Scene *scene, float dt)
    {
        Entity camera_entity = scene->get_primary_camera();
        if (camera_entity)
        {
            auto &camera_transform = camera_entity.get_transform();
            glm::vec3 pos = camera_transform.position;
            glm::vec3 fwd = camera_transform.get_forward();
            glm::vec3 up = camera_transform.get_up();
            Audio::set_listener_position(pos, fwd, up);
        }
        auto view = scene->get_raw_registry().view<AudioSourceComponent, TransformComponent>();
        for (auto entity: view)
        {
            auto &audio_src = view.get<AudioSourceComponent>(entity);
            auto &transform = view.get<TransformComponent>(entity);

            if (audio_src.sound_instance && audio_src.spatialized)
            {
                ma_sound_set_position(audio_src.sound_instance, transform.position.x, transform.position.y,
                                      transform.position.z);
            }

            if (audio_src.sound_instance)
            {
                audio_src.is_playing = ma_sound_is_playing(audio_src.sound_instance);
            }
        }
    }
}
