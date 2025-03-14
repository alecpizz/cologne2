//
// Created by alecpizz on 3/1/2025.
//

#include "EventManager.h"
#include "gpch.h"
#include "Input.h"

namespace goon
{
    struct EventManager::Impl
    {
        bool should_quit = false;
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
            }
            if (event.type == SDL_EVENT_KEY_UP)
            {
                Input::update_key_up(static_cast<uint32_t>(event.key.scancode));
            }
            if (event.type == SDL_EVENT_MOUSE_MOTION)
            {
                Input::update_mouse(event.motion.xrel, event.motion.yrel);
            }
        }
    }

    bool EventManager::should_quit() const
    {
        return _impl->should_quit;
    }

    EventManager::EventManager()
    {
        _impl = new Impl;
    }
}
