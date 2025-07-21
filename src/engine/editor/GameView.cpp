//
// Created by alecpizz on 6/30/25.
//
#include <engine/audio/Audio.h>
#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer.h>

#include "Editor.h"
#include "ImGuizmo.h"

namespace cologne
{
    void Editor::build_game_view(float dt)
    {
        ImGui::Begin("Game View", nullptr, _global_window_flags);
        ImVec2 viewport_size = ImGui::GetContentRegionAvail();
        if (static_cast<int>(_prev_viewport_size.x) != static_cast<int>(viewport_size.x) || static_cast<int>(
                _prev_viewport_size.y) != static_cast<int>(viewport_size.y))
        {
            if (_was_game_mode)
            {
                _was_game_mode = false;
            }
            else
            {
                Engine::get_event_manager()->invoke_resize(viewport_size.x, viewport_size.y);
            }
            _prev_viewport_size = viewport_size;
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
                    Audio::play_sound(_move_sound, 30);
                    Entity temp = {static_cast<entt::entity>(id), Engine::get_scene()};

                    if (temp.has_component<ChildComponent>())
                    {
                        auto comp = temp.get_component<ChildComponent>();
                        if (comp.parent == _selected_entity.get_uuid())
                        {
                            _selected_entity = temp;
                        }
                        else
                        {
                            _selected_entity = Engine::get_scene()->get_entity_by_uuid(comp.parent);
                        }
                    }
                    else
                    {
                        _selected_entity = temp;
                    }
                }
                else
                {
                    _selected_entity = {};
                }
            }
            auto &active = Engine::get_scene()->get_scene_camera().get_component<ActiveComponent>();
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
            {
                _mouse_captured = true;
                active.active = true;
                Engine::get_window()->hide_mouse();
            }
            else
            {
                _mouse_captured = false;
                active.active = false;
                Engine::get_window()->show_mouse();
            }
        }
        else
        {
            if (!_mouse_captured)
            {
                Engine::get_window()->show_mouse();
                auto &active = Engine::get_scene()->get_scene_camera().get_component<ActiveComponent>();
                active.active = false;
            }
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::IsMouseDown(ImGuiMouseButton_Middle))
            {
                _mouse_captured = false;
                auto &active = Engine::get_scene()->get_scene_camera().get_component<ActiveComponent>();
                active.active = false;
                Engine::get_window()->show_mouse();
            }
        }
        ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(Renderer::get_output_image())), viewport_size,
                     ImVec2(0, 1), ImVec2(1, 0));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ENTRY"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                LOG_INFO(" accepted file %s", path);
                auto fs_path = std::filesystem::path(path);
                if (!is_directory(fs_path))
                {
                    if (fs_path.has_extension() && fs_path.extension() == ".glb")
                    {
                        if (fs_path.parent_path().filename().string() == "models")
                        {
                            Engine::get_scene()->create_static_model_entities(
                                fs_path.stem().string().c_str(), TransformComponent(), true);
                        }
                        else if (fs_path.parent_path().filename().string() == "skinned_models")
                        {
                            //todo
                        }
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        static ImGuizmo::OPERATION current_operation = ImGuizmo::OPERATION::TRANSLATE;

        if (ImGui::IsWindowHovered() && !Input::mouse_down(Input::MouseButton::Right))
        {
            const float scroll_speed = 5.0f;
            float scroll = Input::get_scroll().y * dt * scroll_speed;
            auto camera = Engine::get_scene()->get_scene_camera();
            auto &cam_transform = camera.get_transform();
            cam_transform.position = cam_transform.position + cam_transform.get_forward() * scroll;
        }

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
                else if (Input::key_pressed(Input::Key::F))
                {
                    auto camera = Engine::get_scene()->get_scene_camera();
                    auto &cam_transform = camera.get_transform();
                    auto &entity_transform = _selected_entity.get_component<WorldTransformComponent>();
                    cam_transform.position = glm::vec3(entity_transform.transform[3]) - cam_transform.get_forward() *
                                             0.5f;
                }
            }
            auto camera = Engine::get_scene()->get_scene_camera();
            auto camera_comp = camera.get_component<CameraComponent>();
            auto transform = camera.get_transform();
            glm::mat4 view = Renderer::get_camera_view(transform);
            glm::mat4 proj = Renderer::get_camera_projection(transform, camera_comp);

            auto &tr = _selected_entity.get_transform();
            //         auto& tr = Engine::get_scene()->_registry.get<TransformComponent>(entity);
            glm::mat4 mat4 = tr.get_mat4();
            if (_selected_entity.has_component<ChildComponent>())
            {
                auto parent_entity = Engine::get_scene()->get_entity_by_uuid(
                    _selected_entity.get_component<ChildComponent>().parent);
                if (parent_entity)
                {
                    mat4 = parent_entity.get_component<WorldTransformComponent>().transform * mat4;
                }
            }
            bool changed = ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                                                current_operation, ImGuizmo::LOCAL, glm::value_ptr(mat4));
            //
            if (changed)
            {
                glm::mat4 world_mat = mat4;
                if (_selected_entity.has_component<ChildComponent>())
                {
                    auto parent_entity = Engine::get_scene()->get_entity_by_uuid(_selected_entity.get_component<ChildComponent>().parent);
                    mat4 = glm::inverse(
                               parent_entity.get_component<WorldTransformComponent>().transform) * mat4;
                }
                glm::quat orientation;
                glm::vec3 translation;
                glm::vec3 scale;
                glm::vec3 skew;
                glm::vec4 persp;
                glm::decompose(mat4, scale, orientation, translation, skew, persp);
                tr.position = translation;
                tr.rotation = orientation;
                tr.scale = scale;
            }
        }
        ImGui::End();
    }
}
