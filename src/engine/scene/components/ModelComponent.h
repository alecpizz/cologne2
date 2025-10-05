//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/asset_manager/AssetHandle.h>

namespace cologne
{
    struct ModelComponent
    {
        AssetHandle<Model> model;
        bool gi_only = false;
    };
}
