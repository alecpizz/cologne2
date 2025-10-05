//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/core/Color.h>

namespace cologne
{
    struct DecalComponent
    {
        AssetHandle<Texture> albedo;
        AssetHandle<Texture> normal;
        AssetHandle<Texture> orm;
        AssetHandle<Texture> emission;
        Color color_tint = Color(1.0f, 1.0f, 1.0f, 1.0f);
    };
}
