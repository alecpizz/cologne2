/*
                           |  ____|           (_)
   __ _  ___   ___  _ __   | |__   _ __   __ _ _ _ __   ___
  / _` |/ _ \ / _ \| '_ \  |  __| | '_ \ / _` | | '_ \ / _ \
 | (_| | (_) | (_) | | | | | |____| | | | (_| | | | | |  __/
  \__, |\___/ \___/|_| |_| |______|_| |_|\__, |_|_| |_|\___|
   __/ |                                  __/ |
  |___/                                  |___/

*/
#pragma once
#include <cstdint>

namespace goon
{
    class Engine
    {
    public:
        Engine();
        ~Engine();
        Engine(Engine &&) = delete;
        Engine(const Engine &) = delete;
        Engine &operator=(Engine &&) = delete;
        Engine &operator=(const Engine &) = delete;
        bool init(uint32_t width, uint32_t height);
        void run();

    private:
        struct Impl;
        Impl *_impl;
    };
} // goon
