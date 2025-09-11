//
// Created by alecpizz on 8/13/25.
//

#include "RendererSystem.h"

#include <engine/animation/AnimatorComponent.h>
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/RenderItem.h>
#include <engine/scene/Components.h>
#include <engine/scene/Scene.h>

namespace cologne
{
    void RendererSystem::on_update(Scene* scene, float dt)
    {
        //submit draw calls
        auto& registry = scene->get_raw_registry();
        auto view = registry.view<ModelComponent, WorldTransformComponent, ActiveComponent>();
        for (auto entity: view)
        {
            auto [m, tr, active] =
                    view.get<ModelComponent, WorldTransformComponent, ActiveComponent>(entity);
            if (!active)
            {
                continue;
            }
            Model *model = AssetManager::get_model_by_name(m.model_name);
            for (int32_t idx: model->get_mesh_indices())
            {
                Engine::get_renderer()->submit_render_item(
                    RenderItem(idx, tr, m.gi_only, static_cast<uint32_t>(entity)));
            }
        }

        for (auto entity: registry.view<LightComponent, WorldTransformComponent, LightHandleComponent, ActiveComponent>())
        {
            auto [light, transform, handle_comp, active] =
                    registry.get<LightComponent, WorldTransformComponent, LightHandleComponent, ActiveComponent>(entity);
            if (!handle_comp.light_handle.is_valid())
            {
                continue;
            }
            Engine::get_renderer()->update_light_transform(handle_comp.light_handle, TransformComponent(transform));
            Engine::get_renderer()->update_light_properties(handle_comp.light_handle, light, active.active);
        }

        auto view2 = registry.view<MeshComponent, WorldTransformComponent, ActiveComponent>();
        for (auto entity: view2)
        {
            auto [m, tr, active] = view2.get<MeshComponent, WorldTransformComponent, ActiveComponent>(entity);
            if (!active)
            {
                continue;
            }
            Engine::get_renderer()->
                    submit_render_item(RenderItem(AssetManager::get_mesh_index_by_name(m.mesh_name), tr, false,
                                                  static_cast<uint32_t>(entity)));
        }

        auto view3 = registry.view<SkinnedModelComponent, WorldTransformComponent, ActiveComponent>();
        for (auto entity: view3)
        {
            auto [m, tr, active] =
                    view3.get<SkinnedModelComponent, WorldTransformComponent, ActiveComponent>(entity);
            //culling step would be here probably? though for GI i dunno. might have to pack into render item
            if (!active)
            {
                continue;
            }
            SkinnedModel *skinned_model = AssetManager::get_skinned_model_by_name(m.model_name);
            std::vector<glm::mat4> bones;
            if (registry.any_of<AnimatorComponent>(entity))
            {
                auto &animator = registry.get<AnimatorComponent>(entity);
                bones = animator.get_skinning_matrices();
            }
            else if (registry.any_of<AnimComponent>(entity))
            {
                bones = m.skeleton_pose.get_skinning_matrices();
            }
            else
            {
                bones = std::vector<glm::mat4>(skinned_model->get_skeleton().get_bone_count(), glm::mat4(1.0f));
            }
            for (int32_t mesh_index: skinned_model->get_mesh_indices())
            {
                SkinnedRenderItem item;
                item.mesh_idx = mesh_index;
                item.transform = tr;
                item.entity_id = static_cast<uint32_t>(entity);
                item.bones = bones;
                Engine::get_renderer()->submit_skinned_render_item(item);
            }
        }
    }
}
