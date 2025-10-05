//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/animation/Skeleton.h>
#include <engine/animation/SkeletonPose.h>
namespace cologne
{

    struct SkinnedModelComponent
    {
        std::string model_name;

        SkinnedModelComponent(const char *name)
        {
            model_name = name;
        }

        SkinnedModelComponent() = default;

        SkinnedModelComponent(const SkinnedModelComponent &other)
        {
            model_name = other.model_name;
        }

        //runtime
        Skeleton skeleton;
        SkeletonPose skeleton_pose;
    };
}