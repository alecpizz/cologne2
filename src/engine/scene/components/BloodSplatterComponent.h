//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/asset_manager/AssetHandle.h>

namespace cologne
{
    struct BloodSplatterComponent
    {
        AssetHandle<Mesh> mesh;
        AssetHandle<Texture> position_texture;
        AssetHandle<Texture> normal_texture;
        glm::vec3 offset = glm::vec3(0.0f);
        float time = 0.0f;
    };
}
