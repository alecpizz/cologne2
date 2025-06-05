#pragma once
#include "ScriptableEntity.h"

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

        glm::vec3 get_fwd() const
        {
            return glm::vec3(0.0f, 0.0f, 1.0f) * rotation;
        }

        glm::vec3 get_up() const
        {
            return glm::vec3(0.0f, 1.0f, 0.0f) * rotation;
        }

        glm::vec3 get_right() const
        {
            return glm::vec3(1.0f, 0.0f, 0.0f) * rotation;
        }
    };

    struct ActiveComponent
    {
        bool active = true;
        explicit operator bool() { return active; }
        explicit operator const bool() const { return active; }
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

    struct CameraComponent
    {
        float fov_radians = glm::radians(45.0f);
    };

    struct PlayerComponent
    {
        uint32_t id = -1;
    };

    struct NativeScriptComponent
    {
        ScriptableEntity *instance = nullptr;

        ScriptableEntity* (*instantiate_script)();
        void (*destroy_script)(NativeScriptComponent*);

        template<typename T>
        void bind()
        {
            instantiate_script = []() { return static_cast<ScriptableEntity*> (new T()); };
            destroy_script = [](NativeScriptComponent* nsc) {delete nsc->instance; nsc->instance = nullptr;};
        }
    };
}
