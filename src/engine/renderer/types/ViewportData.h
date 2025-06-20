#pragma once

namespace cologne
{
    struct ViewportData
    {
        glm::mat4 projection = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 view_inverse = glm::mat4(1.0f);
        glm::mat4 projection_view = glm::mat4(1.0f);
        glm::vec4 camera_position = glm::vec4(1.0f);
    };
}
