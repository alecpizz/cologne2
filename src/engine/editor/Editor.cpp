//
// Created by alecpizz on 3/1/2025.
//

#include "Editor.h"

#include <engine/core/Engine.h>
#include <engine/scene/Entity.h>
#include <engine/util/DebugScope.h>

#include "ImGuizmo.h"
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
    bool active = false;
    ImVec2 prev_viewport_size = ImVec2(1280, 720);

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
            Engine::get_window()->show_mouse();
        }
        else
        {
            Engine::get_window()->hide_mouse();
        }
    }

    uint32_t Editor::get_viewport_width()
    {
        return prev_viewport_size.x;
    }

    uint32_t Editor::get_viewport_height()
    {
        return prev_viewport_size.y;
    }

    Editor::Editor()
    {
        active = false;
        cologne::DebugScope scope(__PRETTY_FUNCTION__);
        ImGui::CreateContext();
        imguiThemes::green();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        prev_viewport_size = ImVec2(1280, 720);

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            //style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 0.0f;
            style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
        }
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

    void Editor::build()
    {
        ImGui::Begin("cologne window");

        ImGui::Text("FPS %f", ImGui::GetIO().Framerate);
        size_t counter = 0;
        ImGui::SeparatorText("Entities");
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
            bool entity_delete = false;

            if (ImGui::Button("Delete Entity"))
            {
                entity_delete = true;
            }
            auto &active = e.get_component<ActiveComponent>();
            ImGui::Text("%s", tag.tag.c_str());
            ImGui::Checkbox("Active", &active.active);
            ImGui::SeparatorText("Transform");
            build_transform_entry(tr);
            ImGui::Separator();

            if (e.has_component<ViewmodelComponent>())
            {
                ImGui::SeparatorText("View Model Config");
                auto &vm = e.get_component<ViewmodelComponent>();
                ImGui::DragFloat("smoothing", &vm.smoothing, 0.1f);
                ImGui::DragFloat("amplitude", &vm.amplitude, 0.01f);
                ImGui::DragFloat("frequency", &vm.frequency, 0.1f);
                ImGui::DragFloat("vertical velocity multi", &vm.vertical_velocity_multiplier, 0.01f);
                ImGui::DragFloat("max vertical offset", &vm.max_vertical_offset, 0.01f);
                ImGui::DragFloat("sway multiplier", &vm.sway_multiplier);
                ImGui::DragFloat3("position offset", glm::value_ptr(vm.position_offset));
                ImGui::DragFloat3("euler offset", glm::value_ptr(vm.euler_offset));
                ImGui::Separator();
            }

            if (e.has_component<LightComponent>())
            {
                ImGui::SeparatorText("Light Settings");
                auto &light = e.get_component<LightComponent>();
                ImGui::DragFloat("radius", &light.radius, 0.01f);
                ImGui::DragFloat("strength", &light.strength, 0.01f);
                ImGui::ColorEdit3("color", glm::value_ptr(light.color), ImGuiColorEditFlags_HDR
                                                                        | ImGuiColorEditFlags_Float);
                ImGui::Separator();
            }
            ImGui::PopID();
            if (entity_delete)
            {
                Engine::get_scene()->destroy_entity(e);
            }
        }


        ImGui::SeparatorText("Quick Actions");
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
        // auto window = SDL_GL_GetCurrentWindow();
        // ImGui::BeginMainMenuBar(); {
        //     if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0))
        //     {
        //         auto delta = ImGui::GetMouseDragDelta(0);
        //         if (delta.x != 0.0f && delta.y != 0.0f)
        //         {
        //             int currentWindowX, currentWindowY;
        //             SDL_GetWindowPosition(window, &currentWindowX, &currentWindowY);
        //
        //             int newWindowX = currentWindowX + static_cast<int>(delta.x);
        //             int newWindowY = currentWindowY + static_cast<int>(delta.y);
        //             SDL_SetWindowPosition(window, newWindowX, newWindowY);
        //             ImGui::ResetMouseDragDelta(0);
        //         }
        //     }
        //     auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize("- X []   ").x
        //                  - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
        //     if (posX > ImGui::GetCursorPosX())
        //         ImGui::SetCursorPosX(posX);
        //     if (ImGui::MenuItem("-"))
        //     {
        //         Engine::get_window()->minimize();
        //     }
        //     if (ImGui::MenuItem("[]"))
        //     {
        //         Engine::get_window()->maximize();
        //     }
        //     if (ImGui::MenuItem("x"))
        //     {
        //         Engine::get_event_manager()->set_should_quit(true);
        //     }
        // }
        // ImGui::EndMainMenuBar();
    }

    void Editor::build_transform_entry(TransformComponent &tr)
    {
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

    void Editor::present()
    {
        if (active)
        {
            ImGuiStyle &style = ImGui::GetStyle();
            auto color = style.Colors[ImGuiCol_WindowBg];
            color.w = 1.0f;
            // style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
            static Entity selected_entity = {};
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            if (!Engine::get_window()->mouse_visible())
            {
                window_flags |= ImGuiWindowFlags_NoInputs;
            }
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Dock space", nullptr, window_flags);
            ImGui::PopStyleVar(3);

            ImGuiID dock_space_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dock_space_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Exit"))
                    {
                        Engine::get_event_manager()->set_should_quit(true);
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Settings"))
                {
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            ImGui::Begin("Asset Browser");
            if (ImGui::Button("Asset 1"))
            {
            }
            ImGui::SameLine();
            if (ImGui::Button("Asset 2"))
            {
            }
            ImGui::End();


            ImGui::Begin("Scene Hiearchy");
            if (ImGui::TreeNode("Scene"))
            {
                for (auto entity: Engine::get_scene()->_registry.view<entt::entity>())
                {
                    Entity e = {entity, Engine::get_scene()};
                    bool isSelected = selected_entity == e;
                    if (ImGui::Selectable(e.get_component<TagComponent>().tag.c_str(), isSelected))
                    {
                        selected_entity = e;
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::TreePop();
            }
            ImGui::End();

            ImGui::Begin("Properties");
            if (selected_entity)
            {
                ImGui::Text(selected_entity.get_component<TagComponent>().tag.c_str());
                ImGui::Separator();
                ImGui::Text("Transform");
                build_transform_entry(selected_entity.get_component<TransformComponent>());
                if (selected_entity.has_component<ViewmodelComponent>())
                {
                    ImGui::SeparatorText("View Model Config");
                    auto &vm = selected_entity.get_component<ViewmodelComponent>();
                    ImGui::DragFloat("smoothing", &vm.smoothing, 0.1f);
                    ImGui::DragFloat("amplitude", &vm.amplitude, 0.01f);
                    ImGui::DragFloat("frequency", &vm.frequency, 0.1f);
                    ImGui::DragFloat("vertical velocity multi", &vm.vertical_velocity_multiplier, 0.01f);
                    ImGui::DragFloat("max vertical offset", &vm.max_vertical_offset, 0.01f);
                    ImGui::DragFloat("sway multiplier", &vm.sway_multiplier);
                    ImGui::DragFloat3("position offset", glm::value_ptr(vm.position_offset));
                    ImGui::DragFloat3("euler offset", glm::value_ptr(vm.euler_offset));
                    ImGui::Separator();
                }

                if (selected_entity.has_component<LightComponent>())
                {
                    ImGui::SeparatorText("Light Settings");
                    auto &light = selected_entity.get_component<LightComponent>();
                    ImGui::DragFloat("radius", &light.radius, 0.01f);
                    ImGui::DragFloat("strength", &light.strength, 0.01f);
                    ImGui::ColorEdit3("color", glm::value_ptr(light.color), ImGuiColorEditFlags_HDR
                                                                            | ImGuiColorEditFlags_Float);
                    ImGui::Separator();
                }
            } else
            {
                ImGui::Text("Select an entity");
            }
            ImGui::End();

            ImGui::Begin("Settings");
            ImGui::Text("Settings here");
            static bool test = false;
            ImGui::Checkbox("Feature", &test);
            if (ImGui::Button("Print shit"))
            {
                for (size_t i = 0; i < image_cmds.size(); i++)
                {
                    LOG_INFO("image name %s, idx %d", image_cmds[i].name.c_str(), i);
                }
            }
            ImGui::End();

            ImGui::Begin("Game View");
            ImGui::Text("Game rendered here");
            ImVec2 viewport_size = ImGui::GetContentRegionAvail();
            if (prev_viewport_size.x != viewport_size.x || prev_viewport_size.y != viewport_size.y)
            {
                LOG_INFO("view port resized uwu");
                Engine::get_event_manager()->invoke_resize(viewport_size.x, viewport_size.y);
                prev_viewport_size = viewport_size;
            }
            ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(Renderer::get_output_image())), viewport_size,
                         ImVec2(0, 1), ImVec2(1, 0));
            if (selected_entity)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewport_size.x, viewport_size.y);
                //
                //
                auto camera = Engine::get_scene()->get_scene_camera();
                auto camera_comp = camera.get_component<CameraComponent>();
                auto transform = camera.get_component<TransformComponent>();
                glm::mat4 view = Renderer::get_camera_view(transform);
                glm::mat4 proj = Renderer::get_camera_projection(transform, camera_comp);
                auto &tr = selected_entity.get_component<TransformComponent>();
                //         auto& tr = Engine::get_scene()->_registry.get<TransformComponent>(entity);
                glm::mat4 mat4 = tr.get_mat4();
                ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                                     ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::WORLD, glm::value_ptr(mat4));
                //
                glm::quat orientation;
                glm::vec3 translation;
                glm::vec3 scale;
                glm::vec4 persp;
                glm::vec3 skew;
                glm::decompose(mat4, scale, orientation, translation, skew, persp);
                tr.position = translation;
            }
            ImGui::End();
            ImGui::End();
            ImGui::PopStyleColor();
        } else
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
}
