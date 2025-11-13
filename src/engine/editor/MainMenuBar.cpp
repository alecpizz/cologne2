//
// Created by alecpizz on 8/25/25.
//

#include <engine/renderer/Renderer.h>
#include <engine/core/Engine.h>
#include <imgui.h>
#include <engine/audio/Audio.h>
#include <engine/core/EventManager.h>
#include <engine/core/Window.h>
#include <engine/navigation/Navigation.h>
#include <engine/scene/SceneSaver.h>
#include <misc/cpp/imgui_stdlib.h>
#include <engine/navigation/NavmeshBuilder.h>
#include "Editor.h"

namespace cologne
{
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

    struct FloatCmd
    {
        float &ref;
        std::string name;
    };

    struct IntCmd
    {
        int32_t &ref;
        std::string name;
    };

    struct Vec3Cmd
    {
        glm::vec3 &ref;
        std::string name;
    };



    struct BoolCmd
    {
        bool &ref;
        std::string name;
    };

    struct ButtonCmd
    {
        std::function<void()> action;
        std::string name;
    };

    std::vector<FloatCmd> float_cmds;
    std::vector<IntCmd> int_cmds;
    std::vector<Vec3Cmd> vec3_cmds;

    std::vector<BoolCmd> bool_cmds;
    std::vector<ButtonCmd> button_cmds;

    void Editor::add_float_entry(const char *name, float &value)
    {
        float_cmds.emplace_back(FloatCmd{value, name});
    }

    void Editor::add_int_entry(const char *name, int &value)
    {
        int_cmds.emplace_back(IntCmd{value, name});
    }

    void Editor::add_vec3_entry(const char *name, glm::vec3 &value)
    {
        vec3_cmds.emplace_back(Vec3Cmd{value, name});
    }



    void Editor::add_bool_entry(const char *name, bool &value)
    {
        bool_cmds.emplace_back(BoolCmd{value, name});
    }

    void Editor::add_button(const char *name, std::function<void()> action)
    {
        button_cmds.emplace_back(ButtonCmd{action, name});
    }

    void Editor::update_input(float dt)
    {
        ImGui::SetNextFrameWantCaptureKeyboard(false);
        ImGui::SetNextFrameWantCaptureMouse(false);
    }

    void Editor::build_main_menu_bar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Scene"))
                {
                }
                if (ImGui::MenuItem("Load Scene"))
                {
                    Engine::get_window()->show_file_dialogue_window({{"Scene Files", "cscn"}, {"All Files", "*"}},
                                                                    ASSETS_PATH "scenes",
                                                                    reinterpret_cast<void *>(open_scene_callback));
                    _selected_entity = {};
                }
                if (ImGui::MenuItem("Save Scene"))
                {
                    SceneSaver saver(Engine::get_scene().get());
                    saver.serialize(ASSETS_PATH + std::string("scenes/") + Engine::get_scene()->get_scene_name());
                }
                if (ImGui::MenuItem("Reload Scene"))
                {
                    Engine::load_scene(
                        (ASSETS_PATH + std::string("scenes/") + Engine::get_scene()->get_scene_name()).c_str());
                    _selected_entity = {};
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
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Re-Calculate Bounds"))
                {
                    Engine::get_scene()->re_calculate_bounds();
                }
                if (ImGui::MenuItem("Build Navmesh"))
                {
                    NavMeshBuilder::build_navmesh(Engine::get_scene().get());
                    Navigation::cleanup();
                    Navigation::init_navmesh(Engine::get_scene().get());
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Settings"))
            {
                if (ImGui::MenuItem("Save Layout"))
                {
                    ImGui::SaveIniSettingsToDisk(RESOURCES_PATH "editor/imgui_config.ini");
                }

                if (ImGui::Checkbox("Draw Navmesh Visuals", &_draw_navmesh))
                {
                    Navigation::set_drawing_visibility(_draw_navmesh);
                }

                if (ImGui::MenuItem("Hot reload shaders"))
                {
                    Engine::get_renderer()->reload_shaders();
                    // Audio::play_sound(_accept_sound, 30);
                }

                for (size_t i = 0; i < float_cmds.size(); ++i)
                {
                    ImGui::PushID(i);
                    float value = float_cmds[i].ref;
                    if (ImGui::DragFloat(float_cmds[i].name.c_str(), &value, 0.005f))
                    {
                        float_cmds[i].ref = value;
                    }
                    ImGui::PopID();
                }

                for (size_t i = 0; i < int_cmds.size(); ++i)
                {
                    ImGui::PushID(i);
                    int32_t value = int_cmds[i].ref;
                    if (ImGui::DragInt(int_cmds[i].name.c_str(), &value, 0.005f))
                    {
                        int_cmds[i].ref = value;
                    }
                    ImGui::PopID();
                }

                for (size_t i = 0; i < vec3_cmds.size(); i++)
                {
                    ImGui::PushID(i);
                    glm::vec3 value = vec3_cmds[i].ref;
                    if (ImGui::DragFloat3(vec3_cmds[i].name.c_str(), &value[0], 0.005f))
                    {
                        vec3_cmds[i].ref = value;
                    }
                    ImGui::PopID();
                }

                for (size_t i = 0; i < bool_cmds.size(); i++)
                {
                    ImGui::PushID(i);
                    bool value = bool_cmds[i].ref;
                    if (ImGui::Checkbox(bool_cmds[i].name.c_str(), &value))
                    {
                        bool_cmds[i].ref = value;
                        // Audio::play_sound(_accept_sound, 30);
                    }
                    ImGui::PopID();
                }

                for (size_t i = 0; i < button_cmds.size(); i++)
                {
                    ImGui::PushID(i);
                    if (ImGui::Button(button_cmds[i].name.c_str()))
                    {
                        button_cmds[i].action();
                        // Audio::play_sound(_accept_sound, 30);
                    }
                    ImGui::PopID();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Images Viewer"))
                {
                    _image_window_active = true;
                }
                ImGui::EndMenu();
            }

            ImGui::Text("FPS %.f", ImGui::GetIO().Framerate);
            ImGui::EndMainMenuBar();
        }
    }
}
