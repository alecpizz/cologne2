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

        void print()
        {
            if (Input::key_pressed(Input::Key::J))
            {
                LOG_INFO("Projection %s", glm::to_string(projection).c_str());
                LOG_INFO("View %s", glm::to_string(view).c_str());
                LOG_INFO("View Inverse %s", glm::to_string(view_inverse).c_str());
                LOG_INFO("projection_view %s", glm::to_string(projection_view).c_str());
                LOG_INFO("Position %s", glm::to_string(camera_position).c_str());
            }
        }
    };
}
