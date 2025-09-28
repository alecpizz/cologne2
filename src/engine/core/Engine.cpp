//
// Created by alecpizz on 3/1/2025.
//

#include "Engine.h"

#include <engine/asset_manager/AssetManager.h>
#include <engine/renderer/Renderer.h>

#include "FileWatcher.h"
#include "../physics/Physics.h"
#include "../editor/Editor.h"
#include "Input.h"
#include "../audio/Audio.h"
#include "Time.h"
#include <queue>
#include <engine/util/FileUtil.h>
#include <engine/scene/ComponentRegistry.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <engine/navigation/Navigation.h>
#include <engine/scene/Serialization.h>

#include "EventManager.h"
#include "SceneManager.h"
#include "RuntimeState.h"
#include "Window.h"

namespace cologne
{
    Ref<Window> window = nullptr;
    Ref<Renderer> renderer = nullptr;
    Ref<EventManager> event_manager = nullptr;
    Ref<Editor> editor = nullptr;
    Ref<SceneManager> scene_manager = nullptr;
    Ref<FileWatcher> file_watcher = nullptr;
    std::queue<std::pair<std::filesystem::path, FileStatus> > file_status_queue;
    std::string next_scene = std::string();
    bool running = true;
    bool scene_queued = false;
    RuntimeState current_runtime_state = RuntimeState::EDIT_MODE;
    bool editor_active = true;

    struct ElapsedTime
    {
        std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
        std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
        float elapsed = 0.0f;

        void update()
        {
            end = std::chrono::system_clock::now();
            elapsed = static_cast<std::chrono::duration<float>>(end - start).count();
            start = end;
        }
    };

    Engine::Engine()
    {
        LOG_INFO("Starting up engine. the world is a shit place, and this is a shit engine. good luck!");
        LOG_INFO("C++ VERSION %d", __cplusplus);
    }

    Engine::~Engine()
    {
        scene_manager = nullptr;
        cologne::Audio::destroy();
        cologne::Physics::cleanup();
        renderer = nullptr;
        event_manager = nullptr;
        editor = nullptr;
        file_watcher = nullptr;
        window = nullptr;
    }

    Ref<Renderer> Engine::get_renderer()
    {
        return renderer;
    }

    Ref<Window> Engine::get_window()
    {
        return window;
    }

    Ref<EventManager> Engine::get_event_manager()
    {
        return event_manager;
    }

    Ref<Scene> Engine::get_scene()
    {
        return scene_manager->get_active_scene();
    }


    Ref<Editor> Engine::get_editor()
    {
        return editor;
    }

    Ref<SceneManager> Engine::get_scene_manager()
    {
        return scene_manager;
    }

    static void file_changed(std::filesystem::path path, FileStatus status)
    {
        std::string status_str;
        switch (status)
        {
            case FileStatus::CREATED:
                status_str = "CREATED";
                break;
            case FileStatus::MODIFIED:
                status_str = "MODIFIED";
                break;
            case FileStatus::ERASED:
                status_str = "ERASED";
                break;
            default:
                status_str = "";
                break;
        }
        LOG_INFO("file changed %s %s", path.c_str(), status_str.c_str());
    }

    bool Engine::init(uint32_t width, uint32_t height)
    {
        ElapsedTime et;
        Audio::init();
        editor = create_ref<Editor>();
        window = create_ref<Window>(width, height);
        file_watcher = create_ref<FileWatcher>(
            RESOURCES_PATH, [this](const std::filesystem::path &path, FileStatus status)
            {
                file_status_queue.emplace(path, status);
            });
        renderer = create_ref<Renderer>();
        scene_manager = create_ref<SceneManager>();
        Scene::initialize_systems();
        Physics::init();
        AssetManager::init();
        AssetManager::print_all();
        // Audio::add_music(RESOURCES_PATH "sounds/music2.mp3");
        // Audio::play_music(RESOURCES_PATH "sounds/music2.mp3");
        // Audio::set_music_volume(12);
        ComponentRegistry::register_components();
        Serialization::init();
        //TODO: move this to scene manager?
        if (FileUtil::file_exists(RESOURCES_PATH "last_saved_scene.json"))
        {
            std::ifstream file(RESOURCES_PATH "last_saved_scene.json");
            if (file.is_open())
            {
                nlohmann::json j = nlohmann::json::parse(file);
                std::string last_save_path = j["scene_name"];
                LOG_INFO("LOADED PREVIOUSLY USED SCENE %s", last_save_path.c_str());
                scene_manager->set_editor_scene(ASSETS_PATH + std::string("scenes/") + last_save_path);
            }
            file.close();
        }
        else
        {
            scene_manager->set_editor_scene("");
            LOG_INFO("LOADED DEFAULT SCENE");
        }

        event_manager = std::unique_ptr<EventManager>(new EventManager());
        if (window == nullptr || renderer == nullptr)
        {
            LOG_ERROR("Failed to initialize window or renderer!");
            return false;
        }
        LOG_INFO("Engine initialized successfully!");
        et.update();
        LOG_INFO("Engine initialized in %f time", et.elapsed);
        return true;
    }

    void Engine::run()
    {
        running = true;
        ElapsedTime et;
        while (!event_manager->should_quit())
        {
            if (!file_status_queue.empty())
            {
                auto &cmd = file_status_queue.front();
                file_changed(cmd.first, cmd.second);
                if (cmd.second == FileStatus::CREATED)
                {
                    AssetManager::file_added(cmd.first);
                }
                if (cmd.second == FileStatus::MODIFIED)
                {
                    Renderer::file_changed(cmd.first);
                }
                file_status_queue.pop();
            }
            //TODO: move this scene manager
            if (scene_queued)
            {
                Physics::delete_all_bodies();
                if (next_scene.empty())
                {
                    Ref<Scene> scene = create_ref<Scene>();
                    scene->setup_blank_scene();
                    scene_manager->set_editor_scene(scene);
                }
                else
                {
                    scene_manager->set_editor_scene(create_ref<Scene>(next_scene.c_str()));
                }
                scene_queued = false;
            }
            Input::update();
            event_manager->poll_events();
            if (current_runtime_state == RuntimeState::EDIT_MODE)
            {
                scene_manager->get_active_scene()->update_editor(et.elapsed);
            }
            else
            {
                scene_manager->get_active_scene()->update_runtime(et.elapsed);
                Physics::update(et.elapsed);
            }
            Physics::draw();
            Navigation::draw();
            // player->update(et.elapsed);
            window->clear();
            renderer->render_frame();
            if (editor_active)
            {
                editor->present(et.elapsed);
            }
            window->present();
            et.update();
            Time::DeltaTime = et.elapsed;
        }
    }

    void Engine::load_scene(const char *path)
    {
        scene_queued = true;
        next_scene = path;
    }

    void Engine::enter_play_mode()
    {
        if (current_runtime_state == RuntimeState::PLAY_MODE)
        {
            return;
        }
        current_runtime_state = RuntimeState::PLAY_MODE;
        disable_editor();
        event_manager->invoke_resize(window->get_width(), window->get_height());
        scene_manager->on_enter_play_mode();
    }

    void Engine::exit_play_mode()
    {
        if (current_runtime_state == RuntimeState::EDIT_MODE)
        {
            return;
        }
        current_runtime_state = RuntimeState::EDIT_MODE;
        enable_editor();
        scene_manager->on_exit_play_mode();
    }

    void Engine::enable_editor()
    {
        editor_active = true;
        window->show_mouse();
    }

    void Engine::disable_editor()
    {
        editor_active = false;
        window->hide_mouse();
    }

    RuntimeState Engine::get_runtime_state()
    {
        return current_runtime_state;
    }

    uint32_t Engine::get_render_target_width()
    {
        if (Engine::get_runtime_state() == RuntimeState::EDIT_MODE)
        {
            return Editor::get_viewport_width();
        }
        return window->get_width();
    }

    uint32_t Engine::get_render_target_height()
    {
        if (Engine::get_runtime_state() == RuntimeState::EDIT_MODE)
        {
            return Editor::get_viewport_height();
        }
        return window->get_height();
    }

    glm::vec2 Engine::get_render_target_dimensions()
    {
        return glm::vec2(get_render_target_height(), get_render_target_width());
    }
} // cologne
