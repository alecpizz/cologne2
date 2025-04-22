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
#include "DebugUI.h"
#include "Player.h"
#include "engine/renderer/Renderer.h"
#include "engine/Window.h"
#include "engine/Camera.h"
#include "engine/EventManager.h"
#include "engine/GoodGPUs.h"


namespace cologne
{
    class Engine
    {
    public:
        Engine();

        ~Engine();

        static Renderer *get_renderer();

        static Window *get_window();

        static EventManager *get_event_manager();

        static Camera *get_camera();

        static Scene *get_scene();

        static Player *get_player();

        static DebugUI *get_debug_ui();

        Engine(Engine &&) = delete;

        Engine(const Engine &) = delete;

        Engine &operator=(Engine &&) = delete;

        Engine &operator=(const Engine &) = delete;

        bool init(uint32_t width, uint32_t height);

        void run();

    private:
        inline static Engine *_instance;
        struct Impl;
        Impl *_impl;
    };
}
