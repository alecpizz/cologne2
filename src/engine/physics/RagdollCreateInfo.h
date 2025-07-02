#pragma once

namespace cologne
{
    struct BoneInfo
    {
        std::string name;
        glm::vec3 world_space_position = glm::vec3(0.0f);
        glm::quat world_space_rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec3 constraint_pivot_position = glm::vec3(0.0f);
    };

    struct RagdollCreateInfo
    {
        glm::vec3 position;
        std::string hips_name;
        std::string lower_spine_name;
        std::string middle_spine_name;
        std::string upper_spine_name;

        std::string head_name;

        std::string left_arm_name;
        std::string left_fore_arm_name;
        std::string left_hand_name;

        std::string right_arm_name;
        std::string right_fore_arm_name;
        std::string right_hand_name;

        std::string left_up_leg_name;
        std::string left_leg_name;
        std::string left_foot_name;

        std::string right_up_leg_name;
        std::string right_leg_name;
        std::string right_foot_name;
    };
}
