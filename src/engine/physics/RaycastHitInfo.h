#pragma once

namespace cologne
{
    struct RaycastHitInfo
    {
        glm::vec3 origin = glm::vec3(0.0f);
        glm::vec3 direction = glm::vec3(0.0f);
        glm::vec3 hit_point = glm::vec3(0.0f);
        glm::vec3 hit_normal = glm::vec3(0.0f);
        float hit_length = 0.0f;
        Entity hit_entity;
    };
}
