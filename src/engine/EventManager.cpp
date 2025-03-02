//
// Created by alecpizz on 3/1/2025.
//

#include "EventManager.h"
#include "gpch.h"

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
            // ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
            {
                _impl->should_quit = true;
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
