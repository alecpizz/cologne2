//
// Created by alecpizz on 8/20/25.
//

#include "SceneManager.h"

#include <engine/physics/Physics.h>
#include <engine/renderer/Renderer.h>
#include <engine/scene/Scene.h>

#include "Engine.h"

namespace cologne
{
    SceneManager::SceneManager(Ref<Scene> editor_scene)
    {
        set_editor_scene(editor_scene);
    }

    void SceneManager::set_editor_scene(const std::string &path)
    {
        if (!path.empty())
        {
            Ref<Scene> scene = create_ref<Scene>(path.c_str());
            scene->setup_blank_scene();
            set_editor_scene(scene);
        }
        else
        {
            Ref<Scene> scene = create_ref<Scene>();
            scene->setup_blank_scene();
            set_editor_scene(scene);
        }
    }

    void SceneManager::set_editor_scene(Ref<Scene> scene)
    {
        _editor_scene = scene;
        _active_scene = _editor_scene;
    }

    void SceneManager::enter_play_mode()
    {
        //dupe the scene
        _runtime_scene = Scene::copy(_editor_scene);
        _active_scene = _runtime_scene;
        Physics::delete_all_bodies();
        Engine::get_renderer()->clear_lights();
        _active_scene->on_enter_play_mode();
    }

    void SceneManager::exit_play_mode()
    {
        Physics::delete_all_bodies();
        Engine::get_renderer()->clear_lights();
        _active_scene = _editor_scene;
        _runtime_scene = nullptr;
    }

    Ref<Scene> SceneManager::get_active_scene()
    {
        return _active_scene;
    }
}
