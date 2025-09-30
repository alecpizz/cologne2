//
// Created by alecpizz on 9/29/25.
//

#pragma once

namespace cologne
{
    struct Color
    {
        Color() = default;
        Color(float r, float g, float b, float a)
        {
             color = glm::vec4(r, g, b, a);
        }
        glm::vec4 color;
        operator glm::vec4() const { return color; }
    };
}