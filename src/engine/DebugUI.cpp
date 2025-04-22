//
// Created by alecpizz on 3/1/2025.
//

#include "DebugUI.h"
#include "engine/imguiThemes.h"
#include "Engine.h"

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

    struct DebugUI::Impl
    {
        std::vector<FloatCmd> float_cmds;
        std::vector<IntCmd> int_cmds;
        std::vector<Vec3Cmd> vec3_cmds;
        std::vector<ImageCmd> image_cmds;
        std::vector<BoolCmd> bool_cmds;
        std::vector<ButtonCmd> button_cmds;

        void init()
        {
            ImGui::CreateContext();
            imguiThemes::green();
            ImGuiIO &io = ImGui::GetIO();
            (void) io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows


            ImGuiStyle &style = ImGui::GetStyle();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                //style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 0.f;
                style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
            }
        }
    };

    DebugUI::DebugUI()
    {
        _impl = new Impl();
        _impl->init();
    }

    DebugUI::~DebugUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        delete _impl;
    }

    void DebugUI::clear()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);
    }

    void DebugUI::build()
    {
        ImGui::Begin("cologne window");

        ImGui::Text("FPS %f", ImGui::GetIO().Framerate);
        size_t model_count = Engine::get_scene()->get_model_count();
        for (size_t i = 0; i < model_count; i++)
        {
            ImGui::PushID(i);
            auto model = Engine::get_scene()->get_model_by_index(i);
            glm::vec3 scale = model->get_transform()->scale;
            glm::quat rotation = model->get_transform()->rotation;
            glm::vec3 translation = model->get_transform()->translation;

            glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));

            ImGui::LabelText("%s", model->get_path());

            if (ImGui::DragFloat3("Position", glm::value_ptr(translation)))
            {
                model->get_transform()->set_translation(translation);
            }
            if (ImGui::DragFloat3("Euler", glm::value_ptr(euler)))
            {
            }
            if (ImGui::DragFloat3("Scale", glm::value_ptr(scale)))
            {
                model->get_transform()->set_scale(scale);
            }

            bool active = model->get_active();
            if (ImGui::Checkbox("Active", &active))
            {
                model->set_active(active);
            }
            ImGui::PopID();
        }

        if (ImGui::Button("Hot reload shaders"))
        {
            Engine::get_renderer()->reload_shaders();
        }

        for (size_t i = 0; i < _impl->float_cmds.size(); ++i)
        {
            ImGui::PushID(i);
            float value = _impl->float_cmds[i].ref;
            if (ImGui::DragFloat(_impl->float_cmds[i].name.c_str(), &value, 0.005f))
            {
                _impl->float_cmds[i].ref = value;
            }
            ImGui::PopID();
        }

        for (size_t i = 0; i < _impl->int_cmds.size(); ++i)
        {
            ImGui::PushID(i);
            int32_t value = _impl->int_cmds[i].ref;
            if (ImGui::DragInt(_impl->int_cmds[i].name.c_str(), &value, 0.005f))
            {
                _impl->int_cmds[i].ref = value;
            }
            ImGui::PopID();
        }

        for (size_t i = 0; i < _impl->vec3_cmds.size(); i++)
        {
            ImGui::PushID(i);
            glm::vec3 value = _impl->vec3_cmds[i].ref;
            if (ImGui::DragFloat3(_impl->vec3_cmds[i].name.c_str(), &value[0], 0.005f))
            {
                _impl->vec3_cmds[i].ref = value;
            }
            ImGui::PopID();
        }

        for (size_t i = 0; i < _impl->bool_cmds.size(); i++)
        {
            ImGui::PushID(i);
            bool value = _impl->bool_cmds[i].ref;
            if (ImGui::Checkbox(_impl->bool_cmds[i].name.c_str(), &value))
            {
                _impl->bool_cmds[i].ref = value;
            }
            ImGui::PopID();
        }

        for (size_t i = 0; i < _impl->button_cmds.size(); i++)
        {
            ImGui::PushID(i);
            if (ImGui::Button(_impl->button_cmds[i].name.c_str()))
            {
                _impl->button_cmds[i].action();
            }
            ImGui::PopID();
        }

        if (ImGui::CollapsingHeader("Images"))
        {
            ImGui::BeginChild("Images", ImVec2(0, 800));
            for (size_t i = 0; i < _impl->image_cmds.size(); i++)
            {
                ImGui::PushID(i);
                ImGui::Text(_impl->image_cmds[i].name.c_str());
                ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(_impl->image_cmds[i].id)),
                             ImVec2(_impl->image_cmds[i].image_size.x / 4,
                                    _impl->image_cmds[i].image_size.y / 4),
                             ImVec2(0, 1), ImVec2(1, 0));
                ImGui::PopID();
            }
            ImGui::EndChild();
        }

        bool free_cam = Engine::get_camera()->is_free_cam();
        if (ImGui::Checkbox("Free Cam", &free_cam))
        {
            Engine::get_camera()->set_free_cam(free_cam);
        }

        glm::vec3 dir_light = Engine::get_renderer()->get_directional_light().direction;
        glm::vec3 dir_light_pos = Engine::get_renderer()->get_directional_light().position;;
        if (ImGui::DragFloat3("Directional light direction", glm::value_ptr(dir_light), 0.01f))
        {
            Engine::get_renderer()->set_directional_light(dir_light_pos, dir_light);
        }
        if (ImGui::DragFloat3("Directional light position", glm::value_ptr(dir_light_pos), 0.01f))
        {
            Engine::get_renderer()->set_directional_light(dir_light_pos, dir_light);
        }
        glm::vec3 cam_pos = Engine::get_camera()->get_position();
        glm::vec3 cam_fwd = Engine::get_camera()->get_forward();
        if (ImGui::Button("Align dir light to camera"))
        {
            Engine::get_renderer()->set_directional_light(cam_pos, cam_fwd);
        }

        ImGui::InputFloat3("Camera Position", glm::value_ptr(cam_pos));
        ImGui::InputFloat3("Camera Forward", glm::value_ptr(cam_fwd));

        ImGui::End();
        auto window = SDL_GL_GetCurrentWindow();
        ImGui::BeginMainMenuBar();
        {
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0))
            {
                auto delta = ImGui::GetMouseDragDelta(0);
                if (delta.x != 0.0f && delta.y != 0.0f)
                {
                    int currentWindowX, currentWindowY;
                    SDL_GetWindowPosition(window, &currentWindowX, &currentWindowY);

                    int newWindowX = currentWindowX + static_cast<int>(delta.x);
                    int newWindowY = currentWindowY + static_cast<int>(delta.y);
                    SDL_SetWindowPosition(window, newWindowX, newWindowY);
                    ImGui::ResetMouseDragDelta(0);
                }
            }
            auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize("- X []   ").x
                         - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
            if (posX > ImGui::GetCursorPosX())
                ImGui::SetCursorPosX(posX);
            if (ImGui::MenuItem("-"))
            {
                Engine::get_window()->minimize();
            }
            if (ImGui::MenuItem("[]"))
            {
                Engine::get_window()->maximize();
            }
            if (ImGui::MenuItem("x"))
            {
                Engine::get_event_manager()->set_should_quit(true);
            }
        }
        ImGui::EndMainMenuBar();
    }

    void DebugUI::present()
    {
        if (Engine::get_event_manager()->paused())
        {
            build();
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

    void DebugUI::add_float_entry(const char *name, float &value)
    {
        _impl->float_cmds.emplace_back(FloatCmd{value, name});
    }

    void DebugUI::add_int_entry(const char *name, int &value)
    {
        _impl->int_cmds.emplace_back(IntCmd{value, name});
    }

    void DebugUI::add_vec3_entry(const char *name, glm::vec3 &value)
    {
        _impl->vec3_cmds.emplace_back(Vec3Cmd{value, name});
    }

    void DebugUI::add_image_entry(const char *name, uint32_t value, const glm::vec2 &image_size)
    {
        _impl->image_cmds.emplace_back(ImageCmd{value, name, image_size});
    }

    void DebugUI::add_bool_entry(const char *name, bool &value)
    {
        _impl->bool_cmds.emplace_back(BoolCmd{value, name});
    }

    void DebugUI::add_button(const char *name, std::function<void()> action)
    {
        _impl->button_cmds.emplace_back(ButtonCmd{action, name});
    }
}
