//
// Created by alecpizz on 6/1/25.
//

#pragma once
#include <engine/scene/Components.h>
#include <engine/renderer/types/Model.h>
#include <engine/renderer/types/SkinnedModel.h>

namespace cologne
{
    struct RenderItem
    {
        int32_t mesh_idx = 0;
        WorldTransformComponent transform = {};
        bool gi_only = true;
        uint32_t entity_id = entt::null;
        RenderItem() = default;
        RenderItem(int32_t idx, WorldTransformComponent tr, bool gi, uint32_t id)
            : mesh_idx(idx), transform(tr), gi_only(gi), entity_id(id)
        {
        }
    };

    struct SkinnedRenderItem
    {
        int32_t mesh_idx = 0;
        WorldTransformComponent transform = {};
        std::vector<glm::mat4> bones;
        uint32_t entity_id = entt::null;
    };

    struct BloodRenderItem
    {
        BloodSplatterComponent blood_component = {};
        WorldTransformComponent transform = {};
        uint32_t entity_id = entt::null;
    };
}
