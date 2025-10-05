//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/animation/Skeleton.h>
#include <engine/animation/SkeletonPose.h>
#include <engine/asset_manager/AssetHandle.h>

namespace cologne
{

    struct SkinnedModelComponent
    {
        AssetHandle<SkinnedModel> model;

        SkinnedModelComponent() = default;

        SkinnedModelComponent(const SkinnedModelComponent &other)
        {
            model = other.model;
        }

        //runtime
        Skeleton skeleton;
        SkeletonPose skeleton_pose;
    };
}
