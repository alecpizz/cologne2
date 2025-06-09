//
// Created by alecpizz on 3/1/2025.
//

#include "DebugUI.h"

#include <engine/core/Engine.h>
#include <engine/scene/Entity.h>
#include <engine/util/DebugScope.h>

#include "engine/imguiThemes.h"

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


    DebugUI::DebugUI()
    {
        cologne::DebugScope scope(__PRETTY_FUNCTION__);
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

    DebugUI::~DebugUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
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
        size_t counter = 0;
        ImGui::Separator();
        ImGui::Text("Entities");
        for (auto entity: Engine::get_scene()->_registry.view<entt::entity>())
        {
            Entity e = {entity, Engine::get_scene()};
            if (e.has_component<StaticColliderComponent>())
            {
                continue;
            }
            ImGui::PushID(counter++);
            auto &tr = e.get_component<TransformComponent>();
            auto &tag = e.get_component<TagComponent>();
            if (!ImGui::CollapsingHeader(tag.tag.c_str()))
            {
                ImGui::PopID();
                continue;
            }
            auto &active = e.get_component<ActiveComponent>();

            ImGui::Text("%s", tag.tag.c_str());
            ImGui::Checkbox("Active", &active.active);

            build_transform_entry(tr);

            if (e.has_component<ViewmodelComponent>())
            {
                auto &vm = e.get_component<ViewmodelComponent>();
                ImGui::DragFloat("smoothing", &vm.smoothing, 0.1f);
                ImGui::DragFloat("amplitude", &vm.amplitude, 0.01f);
                ImGui::DragFloat("frequency", &vm.frequency, 0.1f);
                ImGui::DragFloat("vertical velocity multi", &vm.vertical_velocity_multiplier, 0.01f);
                ImGui::DragFloat("max vertical offset", &vm.max_vertical_offset, 0.01f);
                ImGui::DragFloat("sway multiplier", &vm.sway_multiplier);
                ImGui::DragFloat3("position offset", glm::value_ptr(vm.position_offset));
                ImGui::DragFloat3("euler offset", glm::value_ptr(vm.euler_offset));
            }

            if (e.has_component<LightComponent>())
            {
                auto &light = e.get_component<LightComponent>();
                ImGui::DragFloat("radius", &light.radius, 0.01f);
                ImGui::DragFloat("strength", &light.strength, 0.01f);
                ImGui::ColorEdit3("color", glm::value_ptr(light.color),ImGuiColorEditFlags_HDR
                                    | ImGuiColorEditFlags_Float );
            }
            // if (e.has_component<ModelComponent>())
            // {
            //     auto& model = e.get_component<ModelComponent>();
            // }
            //
            // if (e.has_component<SkinnedModelComponent>())
            // {
            //     auto& skinned_model = e.get_component<SkinnedModelComponent>();
            // }
            ImGui::PopID();
        }


        if (ImGui::Button("Hot reload shaders"))
        {
            Engine::get_renderer()->reload_shaders();
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
            }
            ImGui::PopID();
        }

        for (size_t i = 0; i < button_cmds.size(); i++)
        {
            ImGui::PushID(i);
            if (ImGui::Button(button_cmds[i].name.c_str()))
            {
                button_cmds[i].action();
            }
            ImGui::PopID();
        }

        if (ImGui::CollapsingHeader("Images"))
        {
            ImGui::BeginChild("Images", ImVec2(0, 800));
            for (size_t i = 0; i < image_cmds.size(); i++)
            {
                ImGui::PushID(i);
                ImGui::Text(image_cmds[i].name.c_str());
                ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(image_cmds[i].id)),
                             ImVec2(image_cmds[i].image_size.x / 4,
                                    image_cmds[i].image_size.y / 4),
                             ImVec2(0, 1), ImVec2(1, 0));
                ImGui::PopID();
            }
            ImGui::EndChild();
        }

        ImGui::End();
        auto window = SDL_GL_GetCurrentWindow();
        ImGui::BeginMainMenuBar(); {
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

    void DebugUI::build_transform_entry(TransformComponent &tr)
    {
        ImGui::Text("Transform");

        ImGui::DragFloat3("Position", glm::value_ptr(tr.position), 0.01f);

        glm::vec3 euler = glm::eulerAngles(tr.rotation);
        euler = glm::degrees(euler);
        ImGui::DragFloat3("Rotation", glm::value_ptr(euler), 0.1f);
        euler.x = fmodf(euler.x, 360.0f);
        euler.y = fmodf(euler.y, 360.0f);
        euler.z = fmodf(euler.z, 360.0f);
        euler = glm::radians(euler);
        tr.rotation = glm::quat(euler);

        ImGui::DragFloat3("Scale", glm::value_ptr(tr.scale), 0.01f);
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
        float_cmds.emplace_back(FloatCmd{value, name});
    }

    void DebugUI::add_int_entry(const char *name, int &value)
    {
        int_cmds.emplace_back(IntCmd{value, name});
    }

    void DebugUI::add_vec3_entry(const char *name, glm::vec3 &value)
    {
        vec3_cmds.emplace_back(Vec3Cmd{value, name});
    }

    void DebugUI::add_image_entry(const char *name, uint32_t value, const glm::vec2 &image_size)
    {
        image_cmds.emplace_back(ImageCmd{value, name, image_size});
    }

    void DebugUI::add_bool_entry(const char *name, bool &value)
    {
        bool_cmds.emplace_back(BoolCmd{value, name});
    }

    void DebugUI::add_button(const char *name, std::function<void()> action)
    {
        button_cmds.emplace_back(ButtonCmd{action, name});
    }
}
