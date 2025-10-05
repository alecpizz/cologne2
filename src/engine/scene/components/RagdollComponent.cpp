//
// Created by alecpizz on 10/5/25.
//
#include "RagdollComponent.h"
#include <engine/physics/Physics.h>
#include <ranges>
namespace cologne
    {
    void RagdollComponent::to_ragdoll()
    {
        if (id == UINT32_MAX)
        {
            return;
        }
        if (current_state != State::ACTIVE)
        {
            Physics::make_ragdoll_active(id);
            current_state = State::ACTIVE;
        }
    }

    void RagdollComponent::to_kinematic()
    {
        if (id == UINT32_MAX)
        {
            return;
        }
        if (current_state != State::KINEMATIC)
        {
            Physics::make_ragdoll_kinematic(id);
            current_state = State::KINEMATIC;
        }
    }

    void RagdollComponent::take_ragdoll_hit(glm::vec3 point, glm::vec3 normal) const
    {
        if (id == UINT32_MAX)
        {
            return;
        }
        if (current_state != State::ACTIVE)
        {
            return;
        }

        uint32_t closest_body = 0;
        float dist = std::numeric_limits<float>::max();
        for (const auto &id: bone_to_ragdoll_map | std::views::values)
        {
            auto transform = Physics::get_rigidbody_transform(id);
            float distance = glm::distance(glm::vec3(transform[3]), point);
            if (distance < dist)
            {
                closest_body = id;
                dist = distance;
            }
        }
        Physics::add_impulse_force_at_position(closest_body, point, -normal * 200.0f);
    }
}