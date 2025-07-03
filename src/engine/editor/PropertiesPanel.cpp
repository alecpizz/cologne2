//
// Created by alecpizz on 6/30/25.
//

#include <engine/renderer/Renderer.h>
#include <engine/animation/AnimationClip.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/scene/Entity.h>
#include <imgui.h>
#include <engine/animation/AnimatorComponent.h>
#include <engine/audio/Audio.h>
#include <engine/renderer/types/Light.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Editor.h"

namespace cologne
{
    template<typename T>
    void remove_component_menu(Entity e)
    {
        if (ImGui::Button("Remove Component"))
        {
            e.remove_component<T>();
            Audio::play_sound(RESOURCES_PATH "sounds/menus/cancel.wav", 30);
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
        ImGui::Begin("Properties", nullptr, _global_window_flags);
        if (_selected_entity)
        {
            auto &tag = _selected_entity.get_component<TagComponent>();
            ImGui::InputText("Tag", &tag.tag);

            ImGui::Text("Entity ID: %d", static_cast<uint32_t>(_selected_entity));
            ImGui::Separator();
            if (ImGui::Checkbox("Active", &_selected_entity.get_component<ActiveComponent>().active))
            {
                Audio::play_sound(_cancel_sound, 30);
            }
            ImGui::Text("Transform");
            build_transform_entry(_selected_entity.get_component<TransformComponent>());
            if (_selected_entity.has_component<ParentComponent>())
            {
                for (auto child: _selected_entity.get_component<ParentComponent>().children)
                {
                    if (child.has_component<MeshComponent>())
                    {
                        Engine::get_renderer()->submit_outline_render_item(RenderItem(
                            child.get_component<MeshComponent>().mesh_idx,
                            child.get_component<
                                WorldTransformComponent>(),
                            false,
                            static_cast<uint32_t>(child)));
                    }
                    if (child.has_component<ModelComponent>())
                    {
                        auto model = AssetManager::get_model_by_index(child.get_component<ModelComponent>().id);
                        for (auto mesh_index: model->get_mesh_indices())
                        {
                            Engine::get_renderer()->submit_outline_render_item(RenderItem(mesh_index,
                                child.get_component<
                                    WorldTransformComponent>(),
                                false,
                                static_cast<uint32_t>(child)));
                        }
                    }

                    if (child.has_component<SkinnedModelComponent>())
                    {
                        SkinnedRenderItem item;
                        item.skinned_model = AssetManager::get_skinned_model_by_index(
                            child.get_component<SkinnedModelComponent>().id);
                        item.transform = child.get_component<WorldTransformComponent>();
                        if (child.has_component<AnimatorComponent>())
                        {
                            auto &anim = child.get_component<AnimatorComponent>();
                            item.bones = anim.get_skinning_matrices();
                        }
                        Engine::get_renderer()->submit_skinned_outline_render_item(item);
                    }
                }
            }
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

            draw_component<LightComponent>("Light", _selected_entity, true, [this](LightComponent &light)
            {
                auto mat = _selected_entity.get_component<WorldTransformComponent>();
                if (light.type == LightComponent::Point)
                {
                    Engine::get_renderer()->draw_sphere(mat.transform[3],
                                                        light.radius, light.color);
                }
                const char *items[] = {"DIRECTIONAL", "POINT", "SPOT"};
                ImGui::Combo("LIGHT TYPE", &light.type, items, 3);
                if (light.type == LightComponent::Spot)
                {
                    ImGui::DragFloat("outer cutoff", &light.outer_cutoff, 0.01f);
                    ImGui::DragFloat("inner cutoff", &light.inner_cutoff, 0.01f);
                }
                ImGui::DragFloat("radius", &light.radius, 0.01f);
                ImGui::DragFloat("strength", &light.strength, 0.01f);
                ImGui::ColorEdit3("color", glm::value_ptr(light.color), ImGuiColorEditFlags_HDR
                                                                        | ImGuiColorEditFlags_Float);
                ImGui::Checkbox("CAST SHADOWS", &light.cast_shadows);
            });

            draw_component<MeshComponent>("Mesh", _selected_entity, true, [this](auto &mesh_comp)
            {
                Engine::get_renderer()->submit_outline_render_item(RenderItem(mesh_comp.mesh_idx,
                                                                              _selected_entity.get_component<
                                                                                  WorldTransformComponent>(),
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
                        _selected_entity.get_component<WorldTransformComponent>(),
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
                item.transform = _selected_entity.get_component<WorldTransformComponent>();
                if (_selected_entity.has_component<AnimatorComponent>())
                {
                    auto &anim = _selected_entity.get_component<AnimatorComponent>();
                    item.bones = anim.get_skinning_matrices();
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

            draw_component<PlayerComponent>("Player", _selected_entity, true, [](PlayerComponent &player)
            {
#define IMGUI_WIDGET(type, name, ...) \
                ImGui::DragFloat(#name, &player.name);
                PLAYER_COMPONENT_FIELDS(IMGUI_WIDGET)
#undef IMGUI_WIDGET
            });


            draw_component<NativeScriptComponent>("Native Script", _selected_entity, true,
                                                  [](auto &script)
                                                  {
                                                      ImGui::TextDisabled("how should these components work lol");
                                                  });

            draw_component<AnimatorComponent>("Animator", _selected_entity, true, [](AnimatorComponent &anim)
            {
                float progress = anim.get_current_progress();
                float total = anim.get_current_clip()->get_duration();
                float percent = progress / total;
                bool is_ragdoll = anim.get_current_state() == AnimatorComponent::State::RAGDOLLING;
                if (ImGui::Button(is_ragdoll ? "To Animated" : "To Ragdoll"))
                {
                    if (is_ragdoll)
                    {
                        anim.to_kinematic();
                    }
                    else
                    {
                        anim.to_ragdoll();
                    }
                }
                ImGui::SliderFloat("Animation Progress", &percent, 0.0f, 1.0f);
            });

            if (ImGui::Button("Add Component"))
            {
                Audio::play_sound(_move_sound, 30);
                ImGui::OpenPopup("AddComponent");
            }

            if (ImGui::BeginPopup("AddComponent"))
            {
                if (!_selected_entity.has_component<LightComponent>() && ImGui::Button("Light Component"))
                {
                    Audio::play_sound(_accept_sound, 30);
                    _selected_entity.add_component<LightComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<ModelComponent>() && ImGui::MenuItem("Model Component"))
                {
                    Audio::play_sound(_accept_sound, 30);
                    _selected_entity.add_component<ModelComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<MeshComponent>() && ImGui::MenuItem("Mesh Component"))
                {
                    Audio::play_sound(_accept_sound, 30);
                    _selected_entity.add_component<MeshComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<SkinnedModelComponent>() && ImGui::MenuItem(
                        "Skinned Model Component"))
                {
                    Audio::play_sound(_accept_sound, 30);
                    _selected_entity.add_component<SkinnedModelComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<AnimatorComponent>() && _selected_entity.has_component<
                        SkinnedModelComponent>() && ImGui::MenuItem("Animator Component"))
                {
                    Audio::play_sound(_accept_sound, 30);
                    // _selected_entity.add_component<AnimatorComponent>(
                    //     AssetManager::get_first_animation_index_with_name(
                    //         AssetManager::get_skinned_model_by_index(
                    //             _selected_entity.get_component<SkinnedModelComponent>().id)->get_name()));
                    LOG_WARN("NOT IMPLEMENTED");
                    ImGui::CloseCurrentPopup();
                }

                if (!_selected_entity.has_component<NativeScriptComponent>() && ImGui::BeginMenu("Native Script"))
                {
                    //TODO
                    Audio::play_sound(_move_sound, 30);
                    if (ImGui::MenuItem("Player Controller"))
                    {
                        Audio::play_sound(_accept_sound, 30);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Editor Camera"))
                    {
                        Audio::play_sound(_accept_sound, 30);
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
}
