//
// Created by alecpizz on 6/1/25.
//

#pragma once

namespace cologne
{
    struct RenderItem
    {
        Model* model;
        SkinnedModel* skinned_model;
        glm::mat4 transform;
    };
}
