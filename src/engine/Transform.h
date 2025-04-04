#pragma once

namespace goon
{
    struct Transform
    {
        glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

        void set_rotation(glm::quat new_rotation)
        {
            rotation = new_rotation;
        }

        void set_rotation(glm::vec3 euler)
        {
            rotation = (glm::quat(euler));
        }

        void set_scale(glm::vec3 new_scale)
        {
            scale = new_scale;
        }

        void set_translation(glm::vec3 new_translation)
        {
            translation = new_translation;
        }

        glm::mat4 get_model_matrix() const
        {
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, translation);
            m *= glm::toMat4(rotation);
            m = glm::scale(m, scale);
            return m;
        }
    };
}
