//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
{
    struct CameraComponent
    {
        float fov_radians = glm::radians(45.0f);
        bool primary = false;
        bool orthographic = false;
        float ortho_zoom = 5.0f;
    };

}