//
// Created by alecpizz on 7/19/25.
//
#include "Components.h"

#include <engine/asset_manager/AssetManager.h>

namespace cologne
{
    MeshComponent::MeshComponent(int idx)
    {
        auto mesh_by_index = AssetManager::get_mesh_by_index(idx);
        if (!mesh_by_index)
        {
            return;
        }
        mesh_name = mesh_by_index->get_name();
    }
}
