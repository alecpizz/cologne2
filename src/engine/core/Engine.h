#pragma once
#include "../scene/Scene.h"
#include "Window.h"
#include "EventManager.h"
#include "engine/GoodGPUs.h"


namespace cologne
{
    class SceneManager;
}

namespace cologne
{
    class Renderer;

    class Engine
    {
    public:
        Engine();

        ~Engine();

        static Ref<Renderer> get_renderer();

        static Ref<Window> get_window();

        static Ref<EventManager> get_event_manager();

        static Ref<Scene> get_scene();

        static Ref<Editor> get_debug_ui();

        static Ref<SceneManager> get_scene_manager();

        Engine(Engine &&) = delete;

        Engine(const Engine &) = delete;

        Engine &operator=(Engine &&) = delete;

        Engine &operator=(const Engine &) = delete;

        bool init(uint32_t width, uint32_t height);

        void run();

        static void load_scene(const char* path);

        static bool in_edit_mode();
    //
    // private:
    //     inline static Engine *_instance;
    //     struct Impl;
    //     Impl *_impl;
    };
}
