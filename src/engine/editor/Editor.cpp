//
// Created by alecpizz on 3/1/2025.
//

#include "Editor.h"

#include <engine/renderer/Renderer.h>
#include <engine/animation/Animation.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include <engine/scene/Entity.h>
#include <engine/util/DebugScope.h>

#include "ImGuizmo.h"
#include "engine/imguiThemes.h"
#include <imgui.h>
#include <engine/animation/Animator.h>
#include <engine/audio/Audio.h>
#include <misc/cpp/imgui_stdlib.h>

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
    bool was_game_mode = true;
    bool mouse_captured = false;
    ImVec2 prev_viewport_size = ImVec2(1280, 720);
    ImGuiWindowFlags global_window_flags;
    const char *move_sound = RESOURCES_PATH "sounds/menus/move.wav";
    const char *accept_sound = RESOURCES_PATH "sounds/menus/accept.wav";
    const char *cancel_sound = RESOURCES_PATH "sounds/menus/cancel.wav";

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
            was_game_mode = true;
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
        was_game_mode = true;
        mouse_captured = false;
        cologne::DebugScope scope(__PRETTY_FUNCTION__);
        ImGui::CreateContext();
        ImGui::LoadIniSettingsFromDisk(RESOURCES_PATH "editor/imgui_config.ini");
        imguiThemes::green();

        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        prev_viewport_size = ImVec2(1280, 720);
        io.FontDefault = io.Fonts->AddFontFromFileTTF(RESOURCES_PATH "fonts/Montserrat-Regular.ttf", 16.0f);

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            //style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 0.0f;
            style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
        }

        Audio::add_sound(accept_sound);
        Audio::add_sound(cancel_sound);
        Audio::add_sound(move_sound);
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
        if (mouse_captured)
        {
            global_window_flags = ImGuiWindowFlags_NoInputs;
            window_flags |= ImGuiWindowFlags_NoInputs;
        }
        else
        {
            global_window_flags = ImGuiWindowFlags_None;
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Dock space", nullptr, window_flags);
        ImGui::PopStyleVar(3);
        ImGuiID dock_space_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dock_space_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }


    void Editor::build_main_menu_bar()
    {
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
                if (ImGui::MenuItem("Save Layout"))
                {
                    ImGui::SaveIniSettingsToDisk(RESOURCES_PATH "editor/imgui_config.ini");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void Editor::build_asset_browser()
    {
        ImGui::Begin("Asset Browser", nullptr, global_window_flags);
        if (ImGui::Button("Asset 1"))
        {
            Audio::play_sound(accept_sound, 30);
        }
        ImGui::SameLine();
        if (ImGui::Button("Asset 2"))
        {
            Audio::play_sound(accept_sound, 30);
        }
        ImGui::End();
    }

    void Editor::build_scene_graph()
    {
        ImGui::Begin("Scene Hiearchy", nullptr, global_window_flags);

        for (auto entity: Engine::get_scene()->_registry.view<entt::entity>())
        {
            Entity e = {entity, Engine::get_scene()};
            auto tag = e.get_component<TagComponent>().tag;
            auto flags = (_selected_entity == e ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
            bool opened = ImGui::TreeNodeEx((void *) (uint64_t) (uint32_t) e, flags, tag.c_str());
            if (ImGui::IsItemClicked())
            {
                Audio::play_sound(move_sound, 30);
                _selected_entity = e;
            }
            if (opened)
            {
                auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                bool opened = ImGui::TreeNodeEx((void *) 9817239, flags, tag.c_str());
                if (opened)
                {
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            // bool isSelected = _selected_entity == e;
            // if (ImGui::Selectable(e.get_component<TagComponent>().tag.c_str(), isSelected))
            // {
            //     _selected_entity = e;
            // }
            // if (isSelected)
            // {
            //     ImGui::SetItemDefaultFocus();
            // }
        }

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Create Empty Entity"))
            {
                Audio::play_sound(accept_sound, 30);
                Engine::get_scene()->create_entity("Empty Entity");
            }
            if (ImGui::BeginMenu("Create Static Model Entity"))
            {
                for (auto& model : AssetManager::get_models())
                {
                    if (ImGui::MenuItem(model.get_name()))
                    {
                        Audio::play_sound(accept_sound, 30);
                        Engine::get_scene()->create_static_model_entities(model.get_name(), {}, true);
                    }
                }
                ImGui::EndMenu();
            }
            if (_selected_entity && ImGui::MenuItem("Delete Entity"))
            {
                Engine::get_scene()->destroy_entity(_selected_entity);
                _selected_entity = {};
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    template<typename T>
    void remove_component_menu(Entity e)
    {
        if (ImGui::Button("Remove Component"))
        {
            e.remove_component<T>();
            Audio::play_sound(cancel_sound, 30);
        }
    }

    template<typename T, typename UIFunction>
    static void draw_component(const std::string &name, Entity entity, bool remove, UIFunction ui_function)
    {
        constexpr ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen |
                                                       ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
                                                       ImGuiTreeNodeFlags_AllowItemOverlap |
                                                       ImGuiTreeNodeFlags_FramePadding;
        if (entity.has_component<T>())
        {
            auto &component = entity.get_component<T>();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
            bool open = ImGui::TreeNodeEx((void *) typeid(T).hash_code(), tree_node_flags, name.c_str());
            ImGui::PopStyleVar();

            if (open)
            {
                if (remove)
                {
                    remove_component_menu<T>(entity);
                }
                ImGui::Unindent();
                ui_function(component);
                ImGui::Indent();
                ImGui::TreePop();
            }
        }
    }

    void Editor::build_properties_panel()
    {
        ImGui::Begin("Properties", nullptr, global_window_flags);
        if (_selected_entity)
        {
            auto &tag = _selected_entity.get_component<TagComponent>();
            ImGui::InputText("Tag", &tag.tag);

            ImGui::Text("Entity ID: %d", static_cast<uint32_t>(_selected_entity));
            ImGui::Separator();
            if (ImGui::Checkbox("Active", &_selected_entity.get_component<ActiveComponent>().active))
            {
                Audio::play_sound(cancel_sound, 30);
            }
            ImGui::Text("Transform");
            build_transform_entry(_selected_entity.get_component<TransformComponent>());
            draw_component<ViewmodelComponent>("View Model", _selected_entity, true, [](auto &vm)
            {
                ImGui::DragFloat("smoothing", &vm.smoothing, 0.1f);
                ImGui::DragFloat("amplitude", &vm.amplitude, 0.01f);
                ImGui::DragFloat("frequency", &vm.frequency, 0.1f);
                ImGui::DragFloat("vertical velocity multi", &vm.vertical_velocity_multiplier, 0.01f);
                ImGui::DragFloat("max vertical offset", &vm.max_vertical_offset, 0.01f);
                ImGui::DragFloat("sway multiplier", &vm.sway_multiplier);
                ImGui::DragFloat3("position offset", glm::value_ptr(vm.position_offset));
                ImGui::DragFloat3("euler offset", glm::value_ptr(vm.euler_offset));
            });

            draw_component<LightComponent>("Light", _selected_entity, true, [this](auto &light)
            {
                Engine::get_renderer()->draw_sphere(_selected_entity.get_component<TransformComponent>().position,
                                                    light.radius, light.color);
                ImGui::DragFloat("radius", &light.radius, 0.01f);
                ImGui::DragFloat("strength", &light.strength, 0.01f);
                ImGui::ColorEdit3("color", glm::value_ptr(light.color), ImGuiColorEditFlags_HDR
                                                                        | ImGuiColorEditFlags_Float);
            });

            draw_component<MeshComponent>("Mesh", _selected_entity, true, [this](auto &mesh_comp)
            {
                Engine::get_renderer()->submit_outline_render_item(RenderItem(mesh_comp.mesh_idx,
                                                                              _selected_entity.get_component<
                                                                                  TransformComponent>(),
                                                                              false,
                                                                              static_cast<uint32_t>(_selected_entity)));
                int id = mesh_comp.mesh_idx;
                if (ImGui::InputInt("Mesh ID", &id))
                {
                    id = glm::clamp(id, 0, static_cast<int>(AssetManager::get_meshes().size()) - 1);
                    mesh_comp.mesh_idx = id;
                }
                std::string mesh_name = AssetManager::get_mesh_by_index(mesh_comp.mesh_idx)->get_name();
                ImGui::Text("Name %s", mesh_name.c_str());
            });

            draw_component<ModelComponent>("Model", _selected_entity, true, [this](auto &model)
            {
                auto m = AssetManager::get_model_by_index(model.id);
                for (auto idx: m->get_mesh_indices())
                {
                    Engine::get_renderer()->submit_outline_render_item(RenderItem(
                        idx,
                        _selected_entity.get_component<TransformComponent>(),
                        false,
                        static_cast<uint32_t>(_selected_entity)));
                }
                int id = static_cast<int>(model.id);
                if (ImGui::InputInt("Model ID", &id))
                {
                    id = glm::clamp(id, 0,
                                    static_cast<int>(AssetManager::get_models().size()) - 1);
                    model.id = id;
                }
                std::string model_name = AssetManager::get_model_by_index(model.id)->get_name();
                ImGui::Text("Name %s", model_name.c_str());
                ImGui::Checkbox("GI Only", &model.gi_only);
            });

            draw_component<SkinnedModelComponent>("Skinned Model", _selected_entity, true, [this](auto &model)
            {
                SkinnedRenderItem item;
                item.skinned_model = AssetManager::get_skinned_model_by_index(model.id);
                item.transform = _selected_entity.get_component<TransformComponent>();
                if (_selected_entity.has_component<AnimatorComponent>())
                {
                    auto &anim = _selected_entity.get_component<AnimatorComponent>();
                    item.bones = anim.get_bones();
                }
                Engine::get_renderer()->submit_skinned_outline_render_item(item);
                int id = static_cast<int>(model.id);
                if (ImGui::InputInt("Skinned Model ID", &id))
                {
                    id = glm::clamp(id, 0,
                                    static_cast<int>(AssetManager::get_skinned_models().size()) - 1);
                    model.id = id;
                }
                std::string model_name = AssetManager::get_skinned_model_by_index(model.id)->get_name();
                ImGui::Text("Name %s", model_name.c_str());
            });

            draw_component<StaticColliderComponent>("Static Collider", _selected_entity, false, [](auto &collider)
            {
                ImGui::BeginDisabled(true);
                int id = static_cast<int>(collider.body_id);
                if (ImGui::InputInt("Collider Body ID", &id))
                {
                }
                ImGui::EndDisabled();
            });

            draw_component<CameraComponent>("Camera", _selected_entity, false, [](auto &camera)
            {
                float degrees = glm::degrees(camera.fov_radians);
                if (ImGui::SliderFloat("FOV", &degrees, 30.0f, 120.0f))
                {
                    float radians = glm::radians(degrees);
                    camera.fov_radians = radians;
                }
                ImGui::BeginDisabled(true);

                ImGui::Checkbox("Primary", &camera.primary);
                ImGui::EndDisabled();
            });

            draw_component<PlayerComponent>("Player", _selected_entity, true, [](auto &player)
            {
                ImGui::DragFloat("Character Speed", &player.character_speed);
                ImGui::DragFloat("Jump Speed", &player.jump_speed);
            });


            draw_component<NativeScriptComponent>("Native Script", _selected_entity, true,
                                                  [](auto &script)
                                                  {
                                                      ImGui::TextDisabled("how should these components work lol");
                                                  });

            draw_component<AnimatorComponent>("Animator", _selected_entity, true, [](auto &anim)
            {
                float progress = anim.get_current_time();
                float total = anim.get_current_animation().get_duration();
                float percent = progress / total;
                ImGui::SliderFloat("Animation Progress", &percent, 0.0f, 1.0f);
            });

            if (ImGui::Button("Add Component"))
            {
                Audio::play_sound(move_sound, 30);
                ImGui::OpenPopup("AddComponent");
            }

            if (ImGui::BeginPopup("AddComponent"))
            {
                if (!_selected_entity.has_component<LightComponent>() && ImGui::Button("Light Component"))
                {
                    Audio::play_sound(accept_sound, 30);
                    _selected_entity.add_component<LightComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<ModelComponent>() && ImGui::MenuItem("Model Component"))
                {
                    Audio::play_sound(accept_sound, 30);
                    _selected_entity.add_component<ModelComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<MeshComponent>() && ImGui::MenuItem("Mesh Component"))
                {
                    Audio::play_sound(accept_sound, 30);
                    _selected_entity.add_component<MeshComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<SkinnedModelComponent>() && ImGui::MenuItem(
                        "Skinned Model Component"))
                {
                    Audio::play_sound(accept_sound, 30);
                    _selected_entity.add_component<SkinnedModelComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<AnimatorComponent>() && _selected_entity.has_component<
                        SkinnedModelComponent>() && ImGui::MenuItem("Animator Component"))
                {
                    Audio::play_sound(accept_sound, 30);
                    _selected_entity.add_component<AnimatorComponent>(
                        AssetManager::get_first_animation_index_with_name(
                            AssetManager::get_skinned_model_by_index(
                                _selected_entity.get_component<SkinnedModelComponent>().id)->get_name()));
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<NativeScriptComponent>() && ImGui::BeginMenu("Native Script"))
                {
                    //TODO
                    Audio::play_sound(move_sound, 30);
                    if (ImGui::MenuItem("Player Controller"))
                    {
                        Audio::play_sound(accept_sound, 30);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Editor Camera"))
                    {
                        Audio::play_sound(accept_sound, 30);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }
        }
        else
        {
            ImGui::Text("Select an entity");
        }
        ImGui::End();
    }

    void Editor::build_game_view()
    {
        ImGui::Begin("Game View", nullptr, global_window_flags);
        ImVec2 viewport_size = ImGui::GetContentRegionAvail();
        if (static_cast<int>(prev_viewport_size.x) != static_cast<int>(viewport_size.x) || static_cast<int>(
                prev_viewport_size.y) != static_cast<int>(viewport_size.y))
        {
            if (was_game_mode)
            {
                was_game_mode = false;
            }
            else
            {
                Engine::get_event_manager()->invoke_resize(viewport_size.x, viewport_size.y);
            }
            prev_viewport_size = viewport_size;
        }
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 viewport_pos = ImGui::GetCursorScreenPos();
        ImVec2 mouse_pos_relative = ImVec2(mouse_pos.x - viewport_pos.x, mouse_pos.y - viewport_pos.y);
        mouse_pos_relative.y = viewport_size.y - mouse_pos_relative.y;
        if (mouse_pos_relative.x >= 0 && mouse_pos_relative.y >= 0 && mouse_pos_relative.x < viewport_size.x &&
            mouse_pos_relative.y < viewport_size.y)
        {
            uint32_t x = static_cast<uint32_t>(mouse_pos_relative.x);
            uint32_t y = static_cast<uint32_t>(mouse_pos_relative.y);
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGuizmo::IsOver())
            {
                uint32_t id = Renderer::read_fbo_pixel("gbuffer", "entity_id", x, y);
                if (id != entt::null)
                {
                    Audio::play_sound(move_sound, 30);
                    _selected_entity = {static_cast<entt::entity>(id), Engine::get_scene()};
                }
                else
                {
                    _selected_entity = {};
                }
            }
            auto &active = Engine::get_scene()->get_scene_camera().get_component<ActiveComponent>();
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
            {
                mouse_captured = true;
                active.active = true;
                Engine::get_window()->hide_mouse();
            }
            else
            {
                mouse_captured = false;
                active.active = false;
                Engine::get_window()->show_mouse();
            }
        }
        else
        {
            if (!mouse_captured)
            {
                Engine::get_window()->show_mouse();
                auto &active = Engine::get_scene()->get_scene_camera().get_component<ActiveComponent>();
                active.active = false;
            }
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::IsMouseDown(ImGuiMouseButton_Middle))
            {
                mouse_captured = false;
                auto &active = Engine::get_scene()->get_scene_camera().get_component<ActiveComponent>();
                active.active = false;
                Engine::get_window()->show_mouse();
            }
        }
        ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(Renderer::get_output_image())), viewport_size,
                     ImVec2(0, 1), ImVec2(1, 0));
        static ImGuizmo::OPERATION current_operation = ImGuizmo::OPERATION::TRANSLATE;
        if (_selected_entity)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewport_size.x, viewport_size.y);
            //
            //
            if (ImGui::IsWindowHovered() && !Input::mouse_down(Input::MouseButton::Right))
            {
                if (Input::key_pressed(Input::Key::W))
                {
                    current_operation = ImGuizmo::OPERATION::TRANSLATE;
                }
                else if (Input::key_pressed(Input::Key::E))
                {
                    current_operation = ImGuizmo::OPERATION::ROTATE;
                }
                else if (Input::key_pressed(Input::Key::R))
                {
                    current_operation = ImGuizmo::OPERATION::SCALE;
                }
            }
            auto camera = Engine::get_scene()->get_scene_camera();
            auto camera_comp = camera.get_component<CameraComponent>();
            auto transform = camera.get_component<TransformComponent>();
            glm::mat4 view = Renderer::get_camera_view(transform);
            glm::mat4 proj = Renderer::get_camera_projection(transform, camera_comp);

            auto &tr = _selected_entity.get_component<TransformComponent>();
            //         auto& tr = Engine::get_scene()->_registry.get<TransformComponent>(entity);
            glm::mat4 mat4 = tr.get_mat4();
            bool changed = ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                                                current_operation, ImGuizmo::LOCAL, glm::value_ptr(mat4));
            //
            if (changed)
            {
                glm::quat orientation;
                glm::vec3 translation;
                glm::vec3 scale;
                glm::vec3 skew;
                glm::vec4 persp;
                glm::decompose(mat4, scale, orientation, translation, skew, persp);
                tr.position = translation;
                tr.rotation = orientation;
                tr.scale = scale;
                Physics::sync_transform(_selected_entity);
            }
        }
        ImGui::End();
    }

    void Editor::build_settings_panel()
    {
        ImGui::Begin("Settings", nullptr, global_window_flags);
        if (ImGui::Button("Hot reload shaders"))
        {
            Engine::get_renderer()->reload_shaders();
            Audio::play_sound(accept_sound, 30);
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
                Audio::play_sound(accept_sound, 30);
            }
            ImGui::PopID();
        }

        for (size_t i = 0; i < button_cmds.size(); i++)
        {
            ImGui::PushID(i);
            if (ImGui::Button(button_cmds[i].name.c_str()))
            {
                button_cmds[i].action();
                Audio::play_sound(accept_sound, 30);
            }
            ImGui::PopID();
        }

        static bool was_open = false;
        bool open = ImGui::CollapsingHeader("Images");
        if (was_open != open)
        {
            Audio::play_sound(move_sound, 30);
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


    void Editor::build_transform_entry(TransformComponent &tr)
    {
        ImGui::DragFloat3("Position", glm::value_ptr(tr.position), 0.01f);

        glm::vec3 euler = glm::eulerAngles(tr.rotation);
        euler = glm::degrees(euler);
        ImGui::DragFloat3("Rotation", glm::value_ptr(euler), 0.1f);
        // euler.x = fmodf(euler.x, 360.0f);
        // euler.y = fmodf(euler.y, 360.0f);
        // euler.z = fmodf(euler.z, 360.0f);
        euler = glm::radians(euler);
        tr.rotation = glm::quat(euler);

        ImGui::DragFloat3("Scale", glm::value_ptr(tr.scale), 0.01f);
    }

    void Editor::present()
    {
        if (active)
        {
            build_main_window();
            build_main_menu_bar();
            build_scene_graph();
            build_properties_panel();
            build_settings_panel();
            build_asset_browser();
            build_game_view();
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
}
