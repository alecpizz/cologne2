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
#include "engine/scene/ComponentRegistry.h"

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

    static bool draw_component_editor(entt::entity entity, entt::meta_any instance, entt::meta_custom custom, int& guiID)
    {
        using namespace entt::literals;
        auto meta = instance.type();
        ComponentRegistry::PropertiesMap properties = {};
        if (auto* mp = static_cast<const ComponentRegistry::PropertiesMap*>(custom))
        {
            properties = *mp;
        }

        bool changed = false;
        if (auto write_func = meta.func("editor_write"_hs); write_func)
        {
            // changed |= write_func.invoke(instance, properties).cast<bool>();
            write_func.invoke(instance, properties);
        }
        else if (auto read_func = meta.func("editor_read"_hs); read_func)
        {
            read_func.invoke(instance, properties);
        }
        else if (meta.is_sequence_container())
        {
            bool isOpen = false;
            if (auto it = properties.find("name"_hs); it != properties.end())
            {
                //TODO: ts
            }
            ImGui::Text("TODO: sequence containers");
        }
        else if (meta.is_enum())
        {
            ImGui::Text("TODO: enum");
        }
        else
        {
            for (auto [id, data] : meta.data())
            {
                ImGui::PushID(guiID++);
                ImGui::Indent();
                changed |= draw_component_editor(entity, data.get(instance).as_ref(), data.custom(), guiID);
                ImGui::Unindent();
                ImGui::PopID();
            }
        }
        return changed;
    }

    void Editor::build_properties_panel()
    {
        ImGui::Begin("Properties", nullptr, _global_window_flags);
        if (_selected_entity)
        {
            auto &tag = _selected_entity.get_component<TagComponent>();
            ImGui::InputText("Tag", &tag.tag);

            ImGui::Text("Entity ID: %d", static_cast<uint32_t>(_selected_entity));
            ImGui::Text("Entity UUID %d", static_cast<uint64_t>(_selected_entity.get_uuid()));
            ImGui::Separator();
            if (ImGui::Checkbox("Active", &_selected_entity.get_component<ActiveComponent>().active))
            {
                Audio::play_sound(_cancel_sound, 30);
            }


            for (int i = 0; auto&& [id, storage] : Engine::get_scene()->_registry.storage())
            {
                if (!storage.contains(_selected_entity))
                {
                    continue;
                }
                ImGui::SeparatorText(std::string(storage.type().name()).c_str());
                if (auto meta = entt::resolve(id))
                {
                    draw_component_editor(_selected_entity, meta.from_void(storage.value(_selected_entity)).as_ref(), meta.custom(), i);
                }
            }

            ImGui::Text("Transform");
            build_transform_entry(_selected_entity.get_transform());
            if (_selected_entity.has_component<ParentComponent>())
            {
                for (auto child_id: _selected_entity.get_component<ParentComponent>().children)
                {
                    Entity child = Engine::get_scene()->get_entity_by_uuid(child_id);
                    if (!child)
                    {
                        continue;
                    }
                    if (child.has_component<MeshComponent>())
                    {
                        Engine::get_renderer()->submit_outline_render_item(RenderItem(
                            AssetManager::get_mesh_index_by_name(child.get_component<MeshComponent>().mesh_name),
                            child.get_component<
                                WorldTransformComponent>(),
                            false,
                            static_cast<uint32_t>(child)));
                    }
                    if (child.has_component<ModelComponent>())
                    {
                        auto model = AssetManager::get_model_by_name(child.get_component<ModelComponent>().model_name);
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
                        auto model = AssetManager::get_skinned_model_by_name(
                            child.get_component<SkinnedModelComponent>().model_name);
                        if (model)
                        {
                            std::vector<glm::mat4> bones = std::vector<glm::mat4>();
                            if (child.has_component<AnimatorComponent>())
                            {
                                auto &anim = child.get_component<AnimatorComponent>();
                                bones = anim.get_skinning_matrices();
                            }
                            for (int32_t mesh_index: model->get_mesh_indices())
                            {
                                SkinnedRenderItem item;
                                item.mesh_idx = mesh_index;
                                item.transform = child.get_component<WorldTransformComponent>();
                                item.bones = bones;
                                Engine::get_renderer()->submit_skinned_outline_render_item(item);
                            }
                        }
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
                int type = light.type;
                if (ImGui::Combo("LIGHT TYPE", &type, items, 3))
                {
                    light.type = static_cast<LightComponent::LightType>(type);
                }
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

            draw_component<MeshComponent>("Mesh", _selected_entity, true, [this](MeshComponent &mesh_comp)
            {
                Engine::get_renderer()->submit_outline_render_item(RenderItem(
                    AssetManager::get_mesh_index_by_name(mesh_comp.mesh_name),
                    _selected_entity.get_component<
                        WorldTransformComponent>(),
                    false,
                    static_cast<uint32_t>(_selected_entity)));
                auto mesh = AssetManager::get_mesh_by_name(mesh_comp.mesh_name);
                std::string mesh_name = mesh->get_name();
                ImGui::Text("Name %s Material %d", mesh_name.c_str(), mesh->get_material_index());
                ImGui::Text("Metallic %f Roughness %f", AssetManager::get_material_by_index(mesh->get_material_index())->metallic_override, AssetManager::get_material_by_index(mesh->get_material_index())->roughness_override);
            });

            draw_component<ModelComponent>("Model", _selected_entity, true, [this](ModelComponent &model)
            {
                auto m = AssetManager::get_model_by_name(model.model_name);
                for (auto idx: m->get_mesh_indices())
                {
                    Engine::get_renderer()->submit_outline_render_item(RenderItem(
                        idx,
                        _selected_entity.get_component<WorldTransformComponent>(),
                        false,
                        static_cast<uint32_t>(_selected_entity)));
                }
                ImGui::Text("Name %s", model.model_name.c_str());
                ImGui::Checkbox("GI Only", &model.gi_only);
            });

            draw_component<SkinnedModelComponent>("Skinned Model", _selected_entity, true,
                                                  [this](SkinnedModelComponent &model)
                                                  {
                                                      if (auto skinned_model = AssetManager::get_skinned_model_by_name(
                                                          model.model_name))
                                                      {
                                                          std::vector<glm::mat4> bones = std::vector<glm::mat4>();
                                                          if (_selected_entity.has_component<AnimatorComponent>())
                                                          {
                                                              auto &anim = _selected_entity.get_component<
                                                                  AnimatorComponent>();
                                                              bones = anim.get_skinning_matrices();
                                                          }
                                                          for (int32_t mesh_index: skinned_model->get_mesh_indices())
                                                          {
                                                              SkinnedRenderItem item;
                                                              item.mesh_idx = mesh_index;
                                                              item.transform = _selected_entity.get_component<
                                                                  WorldTransformComponent>();
                                                              item.bones = bones;
                                                              Engine::get_renderer()->
                                                                      submit_skinned_outline_render_item(item);
                                                          }
                                                      }
                                                      ImGui::Text("Name %s", model.model_name.c_str());
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
                ImGui::InputFloat("GRAVITY", &player.gravity);
                ImGui::InputFloat("MOVE SPEED", &player.move_speed);
                ImGui::InputFloat("RUN ACCELERATION", &player.run_acceleration);
                ImGui::InputFloat("RUN DECELERATION", &player.run_deceleration);
                ImGui::InputFloat("AIR ACCELERATION", &player.air_acceleration);
                ImGui::InputFloat("AIR DECELERATION", &player.air_deceleration);
                ImGui::InputFloat("AIR CONTROL", &player.air_control);
                ImGui::InputFloat("SIDE STRAFE ACCELERATION", &player.side_strafe_acceleration);
                ImGui::InputFloat("SIDE STRAFE SPEED", &player.side_strafe_speed);
                ImGui::InputFloat("JUMP SPEED", &player.jump_speed);
                ImGui::InputFloat("FRICTION", &player.friction);
                ImGui::InputFloat("MAX STEP VELOCITY", &player.maxStepVelocity);
                ImGui::InputFloat("MIN STEP VELOCITY", &player.minStepVelocity);
                ImGui::InputFloat("MIN STEP INTERVAL", &player.minStepInterval);
                ImGui::InputFloat("MAX STEP INTERVAL", &player.maxStepInterval);
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
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(euler), 0.1f))
        {
            euler.x = fmodf(euler.x, 360.0f);
            euler.y = fmodf(euler.y, 360.0f);
            euler.z = fmodf(euler.z, 360.0f);
            euler = glm::radians(euler);
            tr.rotation = glm::quat(euler);
        }

        ImGui::DragFloat3("Scale", glm::value_ptr(tr.scale), 0.01f);
    }
}
