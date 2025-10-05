//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
    {
    struct BloodSplatterComponent
    {
        std::string mesh_name;
        std::string position_texture_name;
        std::string normal_texture_name;
        glm::vec3 offset = glm::vec3(0.0f);
        float time = 0.0f;
    };
}