//
// Created by alecpizz on 3/1/2025.
//

#include "EventManager.h"

#include <engine/editor/Editor.h>
#include <engine/renderer/Renderer.h>

#include "Engine.h"
#include "Input.h"

namespace cologne
{
    struct EventManager::Impl
    {
        bool should_quit = false;
        bool paused = false;
    };

    EventManager::~EventManager()
    {
        delete _impl;
    }

    void EventManager::poll_events()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
            {
                _impl->should_quit = true;
            }
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                Input::update_key_down(static_cast<uint32_t>(event.key.scancode));
                if (event.key.scancode == SDL_SCANCODE_TAB)
                {
                    _impl->paused = !_impl->paused;
                    Editor::toggle_edit_mode(_impl->paused);
                    invoke_resize(Engine::get_window()->get_width(), Engine::get_window()->get_height());
                }
            }
            if (event.type == SDL_EVENT_KEY_UP)
            {
                Input::update_key_up(static_cast<uint32_t>(event.key.scancode));
            }
            if (event.type == SDL_EVENT_MOUSE_MOTION)
            {
                Input::update_mouse(event.motion.xrel, event.motion.yrel);
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                Input::update_mouse_down(event.button.button);
            }

            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                Input::update_mouse_up(event.button.button);
            }
        }
    }

    void EventManager::set_should_quit(bool b) const
    {
        _impl->should_quit = b;
    }

    bool EventManager::should_quit() const
    {
        return _impl->should_quit;
    }

    void EventManager::invoke_resize(uint32_t width, uint32_t height)
    {
        Engine::get_window()->resize();
        Engine::get_renderer()->window_resized(width, height);
    }

    bool event_watch(void *data, SDL_Event *event)
    {
        if (event->type == SDL_EVENT_WINDOW_RESIZED)
        {
            Engine::get_window()->resize();
            Engine::get_renderer()->window_resized(event->window.data1, event->window.data2);
            // Engine::get_window()->present();
        }
        return true;
    }

    EventManager::EventManager()
    {
        _impl = new Impl;
        SDL_AddEventWatch(event_watch, nullptr);
        LOG_INFO("Event manager initialized");
    }
}
