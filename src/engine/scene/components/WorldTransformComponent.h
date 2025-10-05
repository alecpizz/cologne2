//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
{
    struct WorldTransformComponent
    {
        glm::mat4 transform;
        operator glm::mat4 &() { return transform; }
        operator const glm::mat4 &() const { return transform; }
    };
}