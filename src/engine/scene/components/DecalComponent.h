//
// Created by alecpizz on 10/5/25.
//
#pragma once
#include <engine/core/Color.h>
namespace cologne
    {
    struct DecalComponent
    {
        std::string albedo_name;
        std::string normal_name;
        std::string orm_name;
        std::string emission_name;
        Color color_tint = Color(1.0f, 1.0f, 1.0f, 1.0f);
    };
}