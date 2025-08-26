#pragma once
#include "engine/GoodGPUs.h"

namespace cologne
{
    class Window;
    class Editor;
    class Scene;
    class SceneManager;
    class EventManager;
}

namespace cologne
{
    enum class RuntimeState;
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

        static Ref<Editor> get_editor();

        static Ref<SceneManager> get_scene_manager();

        Engine(Engine &&) = delete;

        Engine(const Engine &) = delete;

        Engine &operator=(Engine &&) = delete;

        Engine &operator=(const Engine &) = delete;

        bool init(uint32_t width, uint32_t height);

        void run();

        static void load_scene(const char* path);

        static void enter_play_mode();

        static void exit_play_mode();

        static void enable_editor();

        static void disable_editor();

        static RuntimeState get_runtime_state();

        static uint32_t get_render_target_width() ;

        static uint32_t get_render_target_height() ;

        static glm::vec2 get_render_target_dimensions() ;

    //
    // private:
    //     inline static Engine *_instance;
    //     struct Impl;
    //     Impl *_impl;
    };
}
