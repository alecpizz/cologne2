//
// Created by alecpizz on 6/30/25.
//

#include <engine/renderer/Renderer.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/scene/Entity.h>
#include <imgui.h>
#include <engine/audio/Audio.h>
#include <engine/renderer/types/Light.h>
#include <engine/scene/components/ActiveComponent.h>
#include <engine/scene/components/MeshComponent.h>
#include <engine/scene/components/ModelComponent.h>
#include <engine/scene/components/ParentComponent.h>
#include <engine/scene/components/SkinnedModelComponent.h>
#include <engine/scene/components/TagComponent.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Editor.h"
#include "engine/scene/components/ComponentRegistry.h"

namespace cologne
{
    static bool draw_component_editor(entt::entity entity, entt::meta_any instance, entt::meta_custom custom,
                                      int &guiID)
    {
        using namespace entt::literals;
        auto meta = instance.type();
        ComponentRegistry::PropertiesMap properties = {};
        if (auto *mp = static_cast<const ComponentRegistry::PropertiesMap *>(custom))
        {
            properties = *mp;
        }

        bool changed = false;
        if (auto write_func = meta.func("editor_write"_hs); write_func)
        {
            changed |= write_func.invoke(instance, properties).cast<bool>();
            //write_func.invoke(instance, properties);
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
            bool is_open = false;
            //get name of the enum itself
            if (auto it = properties.find("name"_hs); it != properties.end())
            {
                //get preview name of each enum value
                const char *preview = "";
                for (auto [id, data]: meta.data())
                {
                    //this is the right enum value
                    if (instance == data.get({}))
                    {
                        //get preview name of enum value
                        ComponentRegistry::PropertiesMap data_props = {};
                        if (auto *mp = static_cast<const ComponentRegistry::PropertiesMap *>(data.custom()))
                        {
                            data_props = *mp;
                        }
                        if (auto it = data_props.find("name"_hs); it != data_props.end())
                        {
                            auto name = it->second.cast<const char *>();
                            preview = name;
                        }
                        break;
                    }
                }
                is_open = ImGui::BeginCombo(it->second.cast<const char *>(), preview);
            }

            if (is_open)
            {
                //draw all enum values
                for (auto [id, data]: meta.data())
                {
                    ComponentRegistry::PropertiesMap data_props = {};
                    if (auto *mp = static_cast<const ComponentRegistry::PropertiesMap *>(data.custom()))
                    {
                        data_props = *mp;
                    }

                    if (auto it = data_props.find("name"_hs); it != data_props.end())
                    {
                        ImGui::PushID(guiID++);
                        auto name = it->second.cast<const char *>();
                        if (ImGui::Selectable(name, instance == data.get({}), 0))
                        {
                            instance.assign(data.get({}));
                            changed = true;
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::EndCombo();
            }
        }
        else
        {
            for (auto [id, data]: meta.data())
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

    static std::string format_component_name(const std::string &input)
    {
        if (input.empty())
        {
            return "";
        }

        std::string result;
        result.reserve(input.length() * 2);

        result += input[0];

        for (size_t i = 1; i < input.length(); i++)
        {
            if (isupper(input[i]))
            {
                result += ' ';
            }
            result += input[i];
        }

        std::string suffix = " Component";
        if (result.length() >= suffix.length() && result.substr(result.length() - suffix.length()) == suffix)
        {
            result = result.substr(0, result.length() - suffix.length());
        }
        return result;
    }

    static void submit_entity_outlines(Entity entity)
    {
        if (entity.has_component<MeshComponent>())
        {
            auto mesh_comp = entity.get_component<MeshComponent>();
            Engine::get_renderer()->submit_outline_render_item(RenderItem(
                AssetManager::get_mesh_index_by_name(mesh_comp.mesh.handle),
                entity.get_component<
                    WorldTransformComponent>(),
                false,
                static_cast<uint32_t>(entity)));
        }

        if (entity.has_component<ModelComponent>())
        {
            auto model_comp = entity.get_component<ModelComponent>();
            auto m = model_comp.model.get();
            for (auto idx: m->get_mesh_indices())
            {
                Engine::get_renderer()->submit_outline_render_item(RenderItem(
                    idx,
                    entity.get_component<WorldTransformComponent>(),
                    false,
                    static_cast<uint32_t>(entity)));
            }
        }

        if (entity.has_component<SkinnedModelComponent>())
        {
            auto model_comp = entity.get_component<SkinnedModelComponent>();
            if (auto skinned_model = model_comp.model.get())
            {
                for (int32_t mesh_index: skinned_model->get_mesh_indices())
                {
                    SkinnedRenderItem item;
                    item.mesh_idx = mesh_index;
                    item.transform = entity.get_component<
                        WorldTransformComponent>();
                    item.bones = std::vector<
                        glm::mat4>(skinned_model->get_skeleton().get_bone_count(), glm::mat4(1.0f));
                    Engine::get_renderer()->
                            submit_skinned_outline_render_item(item);
                }
            }
        }
    }

    static void submit_entity_gizmos(Entity entity)
    {
        if (entity.has_component<LightComponent>())
        {
            auto mat = entity.get_component<WorldTransformComponent>();
            auto light = entity.get_component<LightComponent>();
            switch (entity.get_component<LightComponent>().type)
            {
                case LightType::Point:
                    Engine::get_renderer()->draw_sphere(mat.transform[3],
                                                        light.radius, light.color);
                    break;
                default:
                    break;
            }
        }

        if (entity.has_component<DecalComponent>())
        {
            auto transform = entity.get_component<WorldTransformComponent>();
            Engine::get_renderer()->draw_box(transform, glm::vec3(-0.5f), glm::vec3(0.5f), glm::vec3(1.0f));
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
            ImGui::Text("Entity UUID %d", static_cast<uint64_t>(_selected_entity.get_uuid()));
            ImGui::Separator();
            if (ImGui::Checkbox("Active", &_selected_entity.get_component<ActiveComponent>().active))
            {
               // Audio::play_sound(_cancel_sound, 30);
            }

            build_transform_entry(_selected_entity.get_transform());

            for (int i = 0; auto &&[id, storage]: Engine::get_scene()->_registry.storage())
            {
                if (!storage.contains(_selected_entity))
                {
                    continue;
                }
                if (!ComponentRegistry::get_component_map().contains(storage.type().hash()))
                {
                    continue;
                }
                auto &type_name = ComponentRegistry::get_component_map().at(storage.type().hash());
                if (auto meta = entt::resolve(entt::hashed_string(type_name.c_str())))
                {
                    auto traits = meta.traits<ComponentRegistry::Traits>();
                    if (traits & ComponentRegistry::Traits::EDITOR_READ_ONLY || traits &
                        ComponentRegistry::Traits::EDITOR_WRITE_ONLY)
                    {
                        constexpr ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen |
                                                                       ImGuiTreeNodeFlags_Framed |
                                                                       ImGuiTreeNodeFlags_SpanAvailWidth |
                                                                       ImGuiTreeNodeFlags_AllowItemOverlap |
                                                                       ImGuiTreeNodeFlags_FramePadding;
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
                        bool open = ImGui::TreeNodeEx(type_name.c_str(), tree_node_flags,
                                                      format_component_name(type_name).c_str());
                        ImGui::PopStyleVar();

                        if (open)
                        {
                            ImGui::Unindent();
                            draw_component_editor(_selected_entity,
                                                  meta.from_void(storage.value(_selected_entity)).as_ref(),
                                                  meta.custom(), i);
                            ImGui::Indent();
                            ImGui::PushID(type_name.c_str());
                            if (ImGui::Button("Remove Component"))
                            {
                                if (auto remove_func = meta.func(entt::hashed_string("remove")); remove_func)
                                {
                                    remove_func.invoke({}, &Engine::get_scene()->_registry,
                                                       static_cast<entt::entity>(_selected_entity));
                                    LOG_INFO("Remove component of type %s", type_name.c_str());
                                }
                            }
                            ImGui::PopID();
                            ImGui::TreePop();
                        }
                    }
                }
            }


            if (_selected_entity.has_component<ParentComponent>())
            {
                for (auto child_id: _selected_entity.get_component<ParentComponent>().children)
                {
                    Entity child = Engine::get_scene()->get_entity_by_uuid(child_id);
                    if (!child)
                    {
                        continue;
                    }
                    submit_entity_outlines(child);
                    submit_entity_gizmos(child);
                }
            }

            submit_entity_outlines(_selected_entity);
            submit_entity_gizmos(_selected_entity);


            if (ImGui::Button("Add Component"))
            {
                //Audio::play_sound(_move_sound, 30);
                ImGui::OpenPopup("AddComponent");
            }

            if (ImGui::BeginPopup("AddComponent"))
            {
                for (const auto &[id, name]: ComponentRegistry::get_component_map())
                {
                    auto storage = Engine::get_scene()->_registry.storage(id);
                    if (storage && storage->contains(_selected_entity))
                    {
                        continue;
                    }

                    auto meta = entt::resolve(
                        entt::hashed_string(ComponentRegistry::get_component_map().at(id).c_str()));
                    if (!meta)
                    {
                        continue;
                    }

                    auto traits = meta.traits<ComponentRegistry::Traits>();
                    if (traits & ComponentRegistry::Traits::EDITOR_READ_ONLY || traits &
                        ComponentRegistry::Traits::EDITOR_WRITE_ONLY)
                    {
                        if (ImGui::MenuItem(ComponentRegistry::get_component_map().at(id).c_str()))
                        {
                            //add component meta
                            auto instance = meta.construct();
                            if (auto emplace_func = instance.type().func(entt::hashed_string("emplace"));
                                emplace_func)
                            {
                                emplace_func.invoke({}, &Engine::get_scene()->_registry,
                                                    static_cast<entt::entity>(_selected_entity), instance.as_ref());
                            }
                        //    Audio::play_sound(_accept_sound, 30);
                            ImGui::CloseCurrentPopup();
                        }
                    }
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
        ImGui::Text("Transform");
        ImGui::DragFloat3("Position", glm::value_ptr(tr.position), 0.01f);

        glm::vec3 euler = glm::degrees(glm::eulerAngles(tr.rotation));
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(euler), 0.1f))
        {
            tr.rotation = glm::quat(glm::radians(euler));
        }

        ImGui::DragFloat3("Scale", glm::value_ptr(tr.scale), 0.01f);
    }
}
