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
        Model *model = nullptr;
        TransformComponent transform = {};
        bool gi_only = true;
        uint32_t entity_id = entt::null;
        RenderItem() = default;
        RenderItem(Model *m, TransformComponent tr, bool gi, uint32_t id)
            : model(m), transform(tr), gi_only(gi), entity_id(id)
        {
        }
    };

    struct SkinnedRenderItem
    {
        SkinnedModel *skinned_model = nullptr;
        TransformComponent transform = {};
        std::vector<glm::mat4> bones;
        uint32_t id = entt::null;
    };
}
