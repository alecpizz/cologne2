#pragma once
#include <engine/physics/Physics.h>
#include <engine/util/Util.h>

#include "ScriptableEntity.h"

namespace cologne
{
    struct WorldTransformComponent
    {
        glm::mat4 transform;
        operator glm::mat4 &() { return transform; }
        operator const glm::mat4 &() const { return transform; }
    };

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
    };

    struct ParentComponent
    {
        std::vector<Entity> children;
    };

    struct ChildComponent
    {
        Entity parent;
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
        int32_t id = 0;
        bool gi_only = false;
    };

    struct MeshComponent
    {
        int32_t mesh_idx = 0;
    };

    struct SkinnedModelComponent
    {
        int32_t id = 0;
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
        bool primary = false;
    };

    struct PlayerComponent
    {
        uint32_t id = -1;
        Entity camera = {};
        Entity viewmodel = {};
        float character_speed = 3.5f;
        float jump_speed = 4.0f;

        void teleport_to_position(glm::vec3 pos)
        {
            cologne::Physics::teleport_player(id, pos);
        }
    };

    struct ViewmodelComponent
    {
        glm::vec3 position_offset = glm::vec3(0.045, -0.270, -0.255);
        glm::vec3 euler_offset = glm::vec3(0.0f, 180.0f, 0.0f);
        float sway_multiplier = 100.0f;
        float smoothing = 8.0f;
        float amplitude = 0.01f;
        float frequency = 15.0f;
        float vertical_velocity_multiplier = 0.01f;
        float max_vertical_offset = 0.05f;
    };

    struct LightComponent
    {
        enum LightType
        {
            Directional = 0,
            Point
        };

        glm::vec3 color = glm::vec4(1, 0.7799999713897705, 0.5289999842643738, 1.0f);
        float strength = 1.0f;
        float radius = 3.0f;
        int type = LightType::Point;
    };


    struct NativeScriptComponent
    {
        ScriptableEntity *instance = nullptr;

        ScriptableEntity * (*instantiate_script)() = nullptr;

        void (*destroy_script)(NativeScriptComponent *) = nullptr;

        template<typename T>
        void bind()
        {
            instantiate_script = []() { return static_cast<ScriptableEntity *>(new T()); };
            destroy_script = [](NativeScriptComponent *nsc)
            {
                delete nsc->instance;
                nsc->instance = nullptr;
            };
        }
    };
}
