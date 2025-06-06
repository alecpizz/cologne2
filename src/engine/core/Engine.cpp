//
// Created by alecpizz on 3/1/2025.
//

#include "Engine.h"

#include <engine/asset_manager/AssetManager.h>

#include "../physics/Physics.h"
#include "../editor/DebugUI.h"
#include "engine/renderer/DebugRenderer.h"
#include "Input.h"
#include "../audio/Audio.h"
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
        cologne::Physics::destroy();
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

    Scene *Engine::get_scene()
    {
        return _instance->_impl->scene.get();
    }


    DebugUI * Engine::get_debug_ui()
    {
        return _instance->_impl->debug_ui.get();
    }

    bool Engine::init(uint32_t width, uint32_t height)
    {
        _impl->debug_ui = std::unique_ptr<DebugUI>(new DebugUI());
        _impl->window = std::unique_ptr<Window>(new Window(width, height));
        Physics::init();
        Audio::init();
        AssetManager::init();
        AssetManager::print_all();
        // Audio::add_music(RESOURCES_PATH "sounds/music2.mp3");
        // Audio::play_music(RESOURCES_PATH "sounds/music2.mp3");
        // Audio::set_music_volume(12);
        _impl->scene = std::make_unique<Scene>();
        _impl->renderer = std::unique_ptr<Renderer>(new Renderer());
        _impl->event_manager = std::unique_ptr<EventManager>(new EventManager());
        if (_impl->window == nullptr || _impl->renderer == nullptr)
        {
            LOG_ERROR("Failed to initialize window or renderer!");
            return false;
        }
        LOG_INFO("Engine initialized successfully!");
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
            _impl->scene->update(et.elapsed);
            // _impl->player->update(et.elapsed);
            Physics::update(et.elapsed);
            _impl->debug_ui->clear();
            _impl->window->clear();
            _impl->renderer->render_scene();
            _impl->debug_ui->present();
            _impl->window->present();
            et.update();
            Time::DeltaTime = et.elapsed;
        }
    }
} // cologne
