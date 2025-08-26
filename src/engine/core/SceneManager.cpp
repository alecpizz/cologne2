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
        Physics::delete_all_bodies();
        Engine::get_renderer()->clear_lights();
        if (_active_scene)
        {
            if (Engine::in_edit_mode())
            {
                _active_scene->on_exit_edit_mode();
            }
            else
            {
                _active_scene->on_exit_play_mode();
            }
        }
        _editor_scene = scene;
        _active_scene = _editor_scene;
        if (Engine::in_edit_mode())
        {
            _active_scene->on_enter_edit_mode();
        }
        else
        {
            _active_scene->on_enter_play_mode();
        }
    }

    void SceneManager::enter_play_mode()
    {
        //dupe the scene
        if (_active_scene)
        {
            _active_scene->on_exit_edit_mode();
        }
        _runtime_scene = Scene::copy(_editor_scene);
        _active_scene = _runtime_scene;
        _active_scene->on_enter_play_mode();
    }

    void SceneManager::exit_play_mode()
    {
        if (_active_scene)
        {
            _active_scene->on_exit_play_mode();
        }
        _runtime_scene = nullptr;
        _active_scene = _editor_scene;
        _active_scene->on_enter_edit_mode();
    }

    Ref<Scene> SceneManager::get_active_scene()
    {
        return _active_scene;
    }
}
