//
// Created by alecpizz on 3/1/2025.
//

#include "Editor.h"

#include <engine/renderer/Renderer.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/util/DebugScope.h>

#include "ImGuizmo.h"
#include "engine/imguiThemes.h"
#include <imgui.h>
#include <engine/audio/Audio.h>
#include <engine/scene/SceneSaver.h>
#include <misc/cpp/imgui_stdlib.h>

namespace cologne
{
    bool active = false;

    bool Editor::in_edit_mode()
    {
        return active;
    }

    void Editor::toggle_edit_mode(bool b)
    {
        active = b;
        //do something with edit mode here
        if (active)
        {
            _was_game_mode = true;
            Engine::get_window()->show_mouse();
        }
        else
        {
            Engine::get_window()->hide_mouse();
        }
    }

    uint32_t Editor::get_viewport_width()
    {
        return _prev_viewport_size.x;
    }

    uint32_t Editor::get_viewport_height()
    {
        return _prev_viewport_size.y;
    }

    ImFont *material_font = nullptr;


    Editor::Editor()
    {
        active = false;
        _was_game_mode = true;
        _mouse_captured = false;
        cologne::DebugScope scope(__PRETTY_FUNCTION__);
        ImGui::CreateContext();
        ImGui::LoadIniSettingsFromDisk(RESOURCES_PATH "editor/imgui_config.ini");
        imguiThemes::green();


        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        _prev_viewport_size = ImVec2(DEFAULT_WIDTH, DEFAULT_HEIGHT
        );
        io.FontDefault = io.Fonts->AddFontFromFileTTF(RESOURCES_PATH "fonts/Montserrat-Regular.ttf", 16.0f);
        material_font = io.Fonts->AddFontFromFileTTF(RESOURCES_PATH "fonts/MaterialIcons-Regular.ttf", 48.0f);
        std::setlocale(LC_CTYPE, ".UTF8");
        ImGuiStyle &style = ImGui::GetStyle();
        style.WindowMenuButtonPosition = ImGuiDir_None;
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            //style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 0.0f;
            style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
        }

        Audio::add_sound(_accept_sound);
        Audio::add_sound(_cancel_sound);
        Audio::add_sound(_move_sound);
        initialize_reflection_editor();
    }

    Editor::~Editor()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void Editor::clear()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);
    }

    void Editor::build_main_window()
    {
        ImGuiStyle &style = ImGui::GetStyle();
        auto color = style.Colors[ImGuiCol_WindowBg];
        color.w = 1.0f;
        // style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        if (_mouse_captured)
        {
            _global_window_flags = ImGuiWindowFlags_NoInputs;
            window_flags |= ImGuiWindowFlags_NoInputs;
        }
        else
        {
            _global_window_flags = ImGuiWindowFlags_None;
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Dock space", nullptr, window_flags);
        ImGui::PopStyleVar(3);
        ImGuiID dock_space_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dock_space_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

    void Editor::handle_hotkeys()
    {
        if (_mouse_captured)
        {
            return;
        }

        auto io = ImGui::GetIO();
        ImGuiKeyChord chord = ImGuiMod_Ctrl | ImGuiKey_S;
        if (ImGui::IsKeyChordPressed(chord))
        {
            SceneSaver saver(Engine::get_scene());
            saver.serialize(RESOURCES_PATH + std::string("scenes/") + Engine::get_scene()->get_scene_name());
        }

        chord = ImGuiKey_Delete;
        if (ImGui::IsKeyChordPressed(chord))
        {
            if (_selected_entity)
            {
                Engine::get_scene()->destroy_entity(_selected_entity);
                _selected_entity = {};
            }
        }
    }

    void open_scene_callback(void *userdata, const char *const*filelist, int filter)
    {
        if (!filelist)
        {
            return;
        }
        const char *file = *filelist;
        if (!file)
        {
            return;
        }
        Engine::load_scene(file);
    }

    void Editor::build_main_menu_bar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Load Scene"))
                {
                    Engine::get_window()->show_file_dialogue_window({{"Scene Files", "cscn"}, {"All Files", "*"}},
                                                                    RESOURCES_PATH "scenes",
                                                                    reinterpret_cast<void *>(open_scene_callback));
                }
                if (ImGui::MenuItem("Save Scene"))
                {
                    SceneSaver saver(Engine::get_scene());
                    saver.serialize(RESOURCES_PATH + std::string("scenes/") + Engine::get_scene()->get_scene_name());
                }
                if (ImGui::MenuItem("Save Scene As"))
                {
                    //open a save file dialogue
                }
                if (ImGui::MenuItem("Exit"))
                {
                    Engine::get_event_manager()->set_should_quit(true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Settings"))
            {
                if (ImGui::MenuItem("Save Layout"))
                {
                    ImGui::SaveIniSettingsToDisk(RESOURCES_PATH "editor/imgui_config.ini");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void Editor::present(float dt)
    {
        if (active)
        {
            handle_hotkeys();
            build_main_window();
            build_main_menu_bar();
            build_scene_graph();
            build_properties_panel();
            build_settings_panel();
            build_asset_browser();
            build_game_view(dt);
            ImGui::End();
            ImGui::PopStyleColor();
        }
        else
        {
            // ImGuiStyle &style = ImGui::GetStyle();
            // style.Colors[ImGuiCol_WindowBg].w = 0.0f;
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        const ImGuiIO &io = ImGui::GetIO();
        (void) io;
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }
    }
}
