#pragma once

namespace cologne
{
    struct ViewportData
    {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 view_inverse;
        glm::mat4 projection_view;
        glm::vec4 camera_position;
    };
}
