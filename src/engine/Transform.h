#pragma once

namespace cologne
{
    struct Transform
    {
        glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::mat4 model_matrix = glm::mat4(1.0f);
        bool dirty = false;

        Transform set_rotation(glm::quat new_rotation)
        {
            rotation = new_rotation;
            dirty = true;
            return *this;
        }

        Transform set_rotation(glm::vec3 euler)
        {
            rotation = (glm::quat(euler));
            dirty = true;
            return *this;
        }

        Transform set_scale(glm::vec3 new_scale)
        {
            scale = new_scale;
            dirty = true;
            return *this;
        }

        Transform set_translation(glm::vec3 new_translation)
        {
            translation = new_translation;
            dirty = true;
            return *this;
        }

        Transform set_model_matrix(glm::mat4 model_mat)
        {
            glm::vec3 scale;
            glm::vec3 translation;
            glm::quat orientation;
            glm::vec3 skew;
            glm::vec4 persp;
            glm::decompose(model_mat, scale, orientation, translation, skew, persp);
            set_translation(translation);
            set_rotation(orientation);
            set_scale(scale);
            model_matrix = model_mat;
            dirty = false;
            return *this;
        }

        glm::mat4 get_model_matrix()
        {
            if (dirty)
            {
                glm::mat4 m = glm::mat4(1.0f);
                m = glm::translate(m, translation);
                m *= glm::toMat4(rotation);
                m = glm::scale(m, scale);
                model_matrix = m;
                dirty = false;
                return m;
            }
            return model_matrix;
        }
    };
}
