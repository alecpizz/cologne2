//
// Created by alecpizz on 6/30/25.
//
#include <engine/asset_manager/AssetManager.h>
#include <engine/audio/Audio.h>
#include <engine/core/Engine.h>
#include <engine/scene/Entity.h>
#include <misc/cpp/imgui_stdlib.h>
#include "Editor.h"

namespace cologne
{
    void Editor::build_scene_graph()
    {
        ImGui::Begin("Scene Hiearchy", nullptr, _global_window_flags);

        std::string scene_name = Engine::get_scene()->get_scene_name();
        if (ImGui::InputText("Scene Name", &scene_name))
        {
            Engine::get_scene()->set_scene_name(scene_name);
        }
        for (auto entity: Engine::get_scene()->_registry.view<entt::entity>())
        {
            Entity e = {entity, Engine::get_scene().get()};
            if (e.has_component<HideInEditorComponent>())
            {
                continue;
            }
            if (!e.has_component<ChildComponent>())
            {
                draw_entity_node(e);
            }
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(
                "GRAPH_ENTITY", ImGuiDragDropFlags_AcceptBeforeDelivery))
            {
                uint32_t id = *(uint32_t *) payload->Data;
                auto scene = Engine::get_scene();
                Entity found_entity = {static_cast<entt::entity>(id), scene.get()};
                UUID found_entity_id = found_entity.get_component<IDComponent>().id;
                //remove the entity from its parent entity
                if (found_entity.has_component<ChildComponent>())
                {
                    auto parent_id = found_entity.get_component<ChildComponent>().parent;
                    auto parent = scene->get_entity_by_uuid(parent_id);
                    auto &vec = parent.get_component<ParentComponent>().children;
                    vec.erase(std::ranges::remove(vec, found_entity_id).begin(), vec.end());
                    found_entity.remove_component<ChildComponent>();
                    LOG_INFO("REPARENTED");
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Create Empty Entity"))
            {
                Audio::play_sound(_accept_sound, 30);
                _selected_entity = Engine::get_scene()->create_entity("Empty Entity");
            }
            if (ImGui::BeginMenu("Create Static Model Entity"))
            {
                for (auto &model: AssetManager::get_models())
                {
                    if (ImGui::MenuItem(model.get_name()))
                    {
                        Audio::play_sound(_accept_sound, 30);
                        _selected_entity = Engine::get_scene()->
                                create_static_model_entities(model.get_name(), {}, true);
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


    void Editor::draw_entity_node(Entity entity)
    {
        if (!entity)
        {
            return;
        }
        auto &tag = entity.get_component<TagComponent>();
        ImGuiTreeNodeFlags flags = ((_selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                                   ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_SpanAvailWidth;
        bool is_child = !entity.has_component<ParentComponent>();
        if (is_child)
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        bool opened = ImGui::TreeNodeEx((void *) (uint64_t) (uint32_t) entity, flags, tag.tag.c_str());
        if (ImGui::IsItemClicked())
        {
            Audio::play_sound(_move_sound, 30);
            _selected_entity = entity;
        }

        if (ImGui::BeginDragDropSource())
        {
            uint32_t id = static_cast<uint32_t>(entity);
            ImGui::SetDragDropPayload("GRAPH_ENTITY", &id, sizeof(uint32_t));
            ImGui::Text("%s", tag.tag.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("GRAPH_ENTITY"))
            {
                uint32_t id = *(uint32_t *) payload->Data;
                Entity found_entity = {static_cast<entt::entity>(id), Engine::get_scene().get()};

                bool bad_parent = found_entity.has_component<ParentComponent>() &&
                                  entity.has_component<ChildComponent>() && entity.get_component<ChildComponent>().
                                  parent == found_entity.get_uuid();
                if (bad_parent)
                {
                    LOG_WARN("You cannot parent an entity to its own child!");
                }
                else
                {
                    //remove the entity from its parent entity
                    if (found_entity.has_component<ChildComponent>())
                    {
                        auto parent_id = found_entity.get_component<ChildComponent>().parent;
                        auto parent_entity = Engine::get_scene()->get_entity_by_uuid(parent_id);
                        auto &vec = parent_entity.get_component<ParentComponent>().children;
                        vec.erase(std::ranges::remove(vec, found_entity.get_uuid()).begin(), vec.end());
                        found_entity.remove_component<ChildComponent>();
                    }

                    //add new child component to the entity
                    found_entity.add_component<ChildComponent>(entity.get_uuid());

                    //add a parent component to this entity
                    if (!entity.has_component<ParentComponent>())
                    {
                        entity.add_component<ParentComponent>();
                    }
                    auto &parent_comp = entity.get_component<ParentComponent>();
                    parent_comp.children.push_back(found_entity.get_uuid());
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (opened)
        {
            if (!is_child)
            {
                auto &children = entity.get_component<ParentComponent>().children;
                for (auto child: children)
                {
                    draw_entity_node(Engine::get_scene()->get_entity_by_uuid(child));
                }
            }
            ImGui::TreePop();
        }
    }
}
