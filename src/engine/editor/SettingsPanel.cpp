//
// Created by alecpizz on 6/30/25.
//
#include <engine/renderer/Renderer.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <imgui.h>
#include <engine/audio/Audio.h>
#include <misc/cpp/imgui_stdlib.h>
#include "Editor.h"

namespace cologne
{
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

    struct ImageCmd
    {
        uint32_t id;
        std::string name;
        glm::vec2 image_size;
        bool flip = true;
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
    std::vector<ImageCmd> image_cmds;
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

    void Editor::add_image_entry(const char *name, uint32_t value, const glm::vec2 &image_size)
    {
        for (auto &image_cmd: image_cmds)
        {
            if (image_cmd.name == name)
            {
                image_cmd.id = value;
                image_cmd.image_size = image_size;
                return;
            }
        }
        image_cmds.emplace_back(ImageCmd{value, name, image_size});
    }

    void Editor::add_bool_entry(const char *name, bool &value)
    {
        bool_cmds.emplace_back(BoolCmd{value, name});
    }

    void Editor::add_button(const char *name, std::function<void()> action)
    {
        button_cmds.emplace_back(ButtonCmd{action, name});
    }

    void Editor::build_settings_panel()
    {
        ImGui::Begin("Settings", nullptr, _global_window_flags);
        if (ImGui::Button("Hot reload shaders"))
        {
            Engine::get_renderer()->reload_shaders();
            Audio::play_sound(_accept_sound, 30);
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
                Audio::play_sound(_accept_sound, 30);
            }
            ImGui::PopID();
        }

        for (size_t i = 0; i < button_cmds.size(); i++)
        {
            ImGui::PushID(i);
            if (ImGui::Button(button_cmds[i].name.c_str()))
            {
                button_cmds[i].action();
                Audio::play_sound(_accept_sound, 30);
            }
            ImGui::PopID();
        }

        static bool was_open = false;
        bool open = ImGui::CollapsingHeader("Images");
        if (was_open != open)
        {
            Audio::play_sound(_move_sound, 30);
        }
        was_open = open;
        if (open)
        {
            ImGui::BeginChild("Images");
            for (size_t i = 0; i < image_cmds.size(); i++)
            {
                ImGui::PushID(i);
                ImGui::Text(image_cmds[i].name.c_str());
                float available_width = ImGui::GetContentRegionAvail().x;
                ImVec2 original_size = ImVec2(image_cmds[i].image_size.x, image_cmds[i].image_size.y);
                ImVec2 new_size = original_size;
                if (original_size.x > 0)
                {
                    float aspect_ratio = original_size.y / original_size.x;
                    new_size.x = available_width;
                    new_size.y = available_width * aspect_ratio;
                }
                ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(image_cmds[i].id)),
                             new_size,
                             ImVec2(0, 1), ImVec2(1, 0));
                ImGui::PopID();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}
