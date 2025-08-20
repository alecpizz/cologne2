//
// Created by alecpizz on 8/20/25.
//
#pragma once

namespace cologne
{
    class Scene;

    class SceneManager
    {
    public:
        SceneManager() = default;
        SceneManager(Ref<Scene> editor_scene);
        void set_editor_scene(Ref<Scene> scene);
        void enter_play_mode();
        void exit_play_mode();
        Ref<Scene> get_active_scene();
    private:
        Ref<Scene> _editor_scene = nullptr;
        Ref<Scene> _runtime_scene = nullptr;
        Ref<Scene> _active_scene = nullptr;
    };
}
