//
// Created by alecpizz on 3/1/2025.
//

#pragma once

namespace cologne
{
    class EventManager
    {
        friend class Engine;

    public:
        ~EventManager();

        EventManager(EventManager &&) = delete;

        EventManager(const EventManager &) = delete;

        EventManager &operator=(EventManager &&) = delete;

        EventManager &operator=(const EventManager &) = delete;

        void poll_events();

        bool should_quit() const;

    private:
        EventManager();

        struct Impl;
        Impl *_impl;
    };
}
