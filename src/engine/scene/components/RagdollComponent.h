//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
{
    struct RagdollComponent
    {
        enum class State
        {
            ACTIVE,
            KINEMATIC
        };

        State current_state = State::KINEMATIC;
        uint32_t id = UINT32_MAX;
        std::unordered_map<std::string, uint32_t> bone_to_ragdoll_map = std::unordered_map<std::string, uint32_t>();

        void to_ragdoll();

        void to_kinematic();

        void take_ragdoll_hit(glm::vec3 point, glm::vec3 normal) const;
    };
}