//
// Created by alecpizz on 3/1/2025.
//

#include "DebugUI.h"
#include "engine/imguiThemes.h"


namespace goon
{
    struct DebugUI::Impl
    {
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
        ImGui::Begin("gooning window");

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

            if (ImGui::InputFloat3("Position", glm::value_ptr(translation)))
            {
                model->get_transform()->set_translation(translation);
            }
            if (ImGui::InputFloat3("Euler", glm::value_ptr(euler)))
            {

            }
            if (ImGui::InputFloat3("Scale", glm::value_ptr(scale)))
            {
                model->get_transform()->set_scale(scale);
            }
            ImGui::PopID();
        }

        if (ImGui::Button("Hot reload shaders"))
        {
            Engine::get_renderer()->reload_shaders();
        }

        glm::vec3 dir_light = Engine::get_renderer()->get_directional_light().direction;
        if (ImGui::InputFloat3("Directional light", glm::value_ptr(dir_light)))
        {
            Engine::get_renderer()->set_directional_light(dir_light);
        }

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
}
