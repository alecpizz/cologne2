#pragma once

namespace cologne
{
    struct TransformComponent
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale = glm::vec3(1.0f);
    };

    struct ModelComponent
    {
        //no clue what i want in here yet lmao, maybe just an id of the list of models? then just load all of da models?
        uint32_t id = 0;
    };

    struct SkinnedModelComponent
    {
        uint32_t id = 0;
    };

    struct TagComponent
    {
        std::string tag = std::string();
    };
}
