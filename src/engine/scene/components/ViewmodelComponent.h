//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
{
    struct ViewmodelComponent
    {
        glm::vec3 position_offset = glm::vec3(0.045, -0.270, -0.2);
        glm::vec3 euler_offset = glm::vec3(0.0f, 180.0f, 0.0f);
        float sway_multiplier = 100.0f;
        float smoothing = 8.0f;
        float amplitude = 0.01f;
        float frequency = 12.0f;
        float vertical_velocity_multiplier = 0.01f;
        float max_vertical_offset = 0.07f;
    };

}