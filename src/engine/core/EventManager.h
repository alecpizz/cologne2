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

        void set_should_quit(bool b) const;

        bool should_quit() const;

        bool paused() const;

    private:
        EventManager();

        struct Impl;
        Impl *_impl;
    };
}
