//
// Created by alecpizz on 3/1/2025.
//

#include "DebugUI.h"
#include "engine/imguiThemes.h"
#include "Engine.h"

namespace goon
{
    struct FloatCmd
    {
        float& ref;
        std::string name;
    };
    struct DebugUI::Impl
    {
        std::vector<FloatCmd> cmds;
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

    void DebugUI::present()
    {
        ImGui::Begin("cologne window");

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

        for (size_t i = 0; i < _impl->cmds.size(); ++i)
        {
            ImGui::PushID(i);
            float value = _impl->cmds[i].ref;
            if (ImGui::DragFloat(_impl->cmds[i].name.c_str(), &value, 0.005f))
            {
                _impl->cmds[i].ref = value;
            }
            ImGui::PopID();
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
        _impl->cmds.emplace_back(FloatCmd{value, name});
    }
}
