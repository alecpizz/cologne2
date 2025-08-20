//
// Created by alecpizz on 8/20/25.
//

#include "SceneManager.h"

namespace cologne
{
    SceneManager::SceneManager(Ref<Scene> editor_scene)
    {
        set_editor_scene(editor_scene);
    }

    void SceneManager::set_editor_scene(Ref<Scene> scene)
    {
        _editor_scene = scene;
        _active_scene = _editor_scene;
    }

    void SceneManager::enter_play_mode()
    {
        //dupe the scene
        assert(false);
        _active_scene = _runtime_scene;
    }

    void SceneManager::exit_play_mode()
    {
        _active_scene = _editor_scene;
    }

    Ref<Scene> SceneManager::get_active_scene()
    {
        return _active_scene;
    }
}
