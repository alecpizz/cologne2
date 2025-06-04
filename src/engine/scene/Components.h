#pragma once

namespace cologne
{
    struct TransformComponent
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale = glm::vec3(1.0f);

        glm::mat4 get_mat4() const
        {
            auto m = glm::mat4(1.0f);
            m = glm::translate(m, position);
            m *= glm::toMat4(rotation);
            m = glm::scale(m, scale);
            return m;
        }
    };

    struct ActiveComponent
    {
        bool active = true;
        explicit operator bool ()  {return active;}
        explicit operator const bool() const {return active;}
    };

    struct ModelComponent
    {
        //no clue what i want in here yet lmao, maybe just an id of the list of models? then just load all of da models?
        size_t id = 0;
        bool gi_only = false;
    };

    struct SkinnedModelComponent
    {
        size_t id = 0;
    };

    struct TagComponent
    {
        std::string tag = std::string();
    };

    struct StaticColliderComponent
    {
        uint32_t body_id = -1;
    };

}
