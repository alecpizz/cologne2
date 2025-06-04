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
        RenderItem() = default;
        RenderItem(Model *m, TransformComponent tr, bool gi)
            : model(m), transform(tr), gi_only(gi)
        {
        }
    };

    struct SkinnedRenderItem
    {
        SkinnedModel *skinned_model = nullptr;
        TransformComponent transform = {};
        std::vector<glm::mat4> bones;
    };
}
