#pragma once

namespace cologne
{
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
