#pragma once

namespace cologne
{
    struct RagdollBone
    {
        uint32_t body_id;
        std::string bone_name;
        glm::mat4 initial_body_to_bone_transform;
    };
}
