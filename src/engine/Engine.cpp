//
// Created by alecpizz on 3/1/2025.
//

#include "Engine.h"

#include "DebugUI.h"
#include "gpch.h"


namespace goon
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

    Engine::Engine()
    {
        _impl = new Impl();
        LOG_INFO("Starting up engine. the world is a shit place, and this is a shit engine. good luck!");
    }

    Engine::~Engine()
    {
        delete _impl;
    }

    Renderer * Engine::get_renderer() const
    {
        return _impl->renderer.get();
    }

    Window * Engine::get_window() const
    {
        return _impl->window.get();
    }

    EventManager * Engine::get_event_manager() const
    {
        return _impl->event_manager.get();
    }

    bool Engine::init(uint32_t width, uint32_t height)
    {
        _impl->debug_ui = std::unique_ptr<DebugUI>(new DebugUI());
        _impl->window = std::unique_ptr<Window>(new Window(width, height));
        _impl->renderer = std::unique_ptr<Renderer>(new Renderer());
        _impl->event_manager = std::unique_ptr<EventManager>(new EventManager());
        _impl->scene = std::make_unique<Scene>();
        if (_impl->window == nullptr || _impl->renderer == nullptr)
        {
            LOG_ERROR("Failed to initialize window or renderer!");
            return false;
        }
        _impl->scene->add_model(RESOURCES_PATH "backpack/backpack.glb");
        auto& model = _impl->scene->get_models()[0];
        model.set_texture(RESOURCES_PATH "backpack/diffuse.jpg", TextureType::ALBEDO);
        return true;
    }

    void Engine::run()
    {
        _impl->running = true;
        while (!_impl->event_manager->should_quit())
        {
            _impl->window->resize();

            _impl->event_manager->poll_events();

            _impl->debug_ui->clear();

            _impl->window->clear();

            _impl->renderer->render_scene(*_impl->scene);

            _impl->debug_ui->present();

            _impl->window->present();
        }
    }
} // goon
