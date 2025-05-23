﻿//
// Created by alecpizz on 3/1/2025.
//

#include "Engine.h"
#include "Physics.h"
#include "DebugUI.h"
#include "engine/renderer/DebugRenderer.h"
#include "Input.h"
#include "Audio.h"
#include "Time.h"

namespace cologne
{
    struct Engine::Impl
    {
        std::unique_ptr<Window> window = nullptr;
        std::unique_ptr<Renderer> renderer = nullptr;
        std::unique_ptr<EventManager> event_manager = nullptr;
        std::unique_ptr<DebugUI> debug_ui = nullptr;
        std::unique_ptr<Scene> scene = nullptr;
        std::unique_ptr<Camera> camera = nullptr;
        std::unique_ptr<Player> player = nullptr;
        bool running = true;
    };

    struct ElapsedTime
    {
        std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
        std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
        float elapsed = 0.0f;

        void update()
        {
            end = std::chrono::system_clock::now();
            elapsed = static_cast<std::chrono::duration<float>>(end - start).count();
            start = end;
        }
    };

    Engine::Engine()
    {
        _instance = this;
        _impl = new Impl();
        LOG_INFO("Starting up engine. the world is a shit place, and this is a shit engine. good luck!");
    }

    Engine::~Engine()
    {
        cologne::physics::destroy();
        cologne::Audio::destroy();
        delete _impl;
    }

    Renderer *Engine::get_renderer()
    {
        return _instance->_impl->renderer.get();
    }

    Window *Engine::get_window()
    {
        return _instance->_impl->window.get();
    }

    EventManager *Engine::get_event_manager()
    {
        return  _instance->_impl->event_manager.get();
    }

    Camera *Engine::get_camera()
    {
        return _instance->_impl->camera.get();
    }

    Scene *Engine::get_scene()
    {
        return _instance->_impl->scene.get();
    }


    Player *Engine::get_player()
    {
        return _instance->_impl->player.get();
    }

    DebugUI * Engine::get_debug_ui()
    {
        return _instance->_impl->debug_ui.get();
    }

    bool Engine::init(uint32_t width, uint32_t height)
    {
        _impl->debug_ui = std::unique_ptr<DebugUI>(new DebugUI());
        _impl->window = std::unique_ptr<Window>(new Window(width, height));
        physics::init();
        Audio::init();
        // Audio::add_music(RESOURCES_PATH "sounds/music2.mp3");
        // Audio::play_music(RESOURCES_PATH "sounds/music2.mp3");
        // Audio::set_music_volume(12);
        _impl->scene = std::make_unique<Scene>();
        _impl->renderer = std::unique_ptr<Renderer>(new Renderer());
        _impl->event_manager = std::unique_ptr<EventManager>(new EventManager());
        _impl->camera = std::make_unique<Camera>(glm::vec3(-5.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                                                 glm::vec3(0.0f, 1.0f, 0.0f));
        _impl->player = std::make_unique<Player>();
        if (_impl->window == nullptr || _impl->renderer == nullptr)
        {
            LOG_ERROR("Failed to initialize window or renderer!");
            return false;
        }

        return true;
    }

    void Engine::run()
    {
        _impl->running = true;
        ElapsedTime et;
        while (!_impl->event_manager->should_quit())
        {
            Input::update();
            _impl->event_manager->poll_events();
            _impl->camera->update(et.elapsed);
            _impl->scene->update(et.elapsed);
            _impl->player->update(et.elapsed);
            physics::update(et.elapsed);
            _impl->debug_ui->clear();
            _impl->window->clear();
            _impl->renderer->render_scene(*_impl->scene);
            _impl->debug_ui->present();
            _impl->window->present();
            et.update();
            Time::DeltaTime = et.elapsed;
        }
    }
} // cologne
