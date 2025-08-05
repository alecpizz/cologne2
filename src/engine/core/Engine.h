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
#include "../scene/Scene.h"
#include "Window.h"
#include "EventManager.h"
#include "engine/GoodGPUs.h"


namespace cologne
{
    class Renderer;

    class Engine
    {
    public:
        Engine();

        ~Engine();

        static Renderer *get_renderer();

        static Window *get_window();

        static EventManager *get_event_manager();

        static Scene *get_scene();


        static Editor *get_debug_ui();

        Engine(Engine &&) = delete;

        Engine(const Engine &) = delete;

        Engine &operator=(Engine &&) = delete;

        Engine &operator=(const Engine &) = delete;

        bool init(uint32_t width, uint32_t height);

        void run();

        static void load_scene(const char* path);

        static bool in_edit_mode();

    private:
        inline static Engine *_instance;
        struct Impl;
        Impl *_impl;
    };
}
