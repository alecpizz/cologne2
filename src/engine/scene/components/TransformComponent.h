//
// Created by alecpizz on 10/5/25.
//

#pragma once
#include <engine/util/Util.h>

namespace cologne
    {
    struct TransformComponent
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale = glm::vec3(1.0f);

        TransformComponent() = default;

        TransformComponent(glm::vec3 pos, glm::quat rot, glm::vec3 sc)
        {
            position = pos;
            rotation = rot;
            scale = sc;
        }

        TransformComponent(glm::mat4 mat)
        {
            glm::vec3 pos, s;
            glm::quat rot;
            Util::decompose_mat4(mat, pos, rot, s);
            position = pos;
            rotation = rot;
            scale = s;
        }

        glm::mat4 get_mat4() const
        {
            auto m = glm::mat4(1.0f);
            m = glm::translate(m, position);
            m *= glm::toMat4(rotation);
            m = glm::scale(m, scale);
            return m;
        }

        glm::vec3 get_forward() const
        {
            return rotation * glm::vec3(0.0f, 0.0f, 1.0f);
        }

        glm::vec3 get_up() const
        {
            return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
        }

        glm::vec3 get_right() const
        {
            return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        }

        bool operator==(const TransformComponent &transform) const = default;
    };
}