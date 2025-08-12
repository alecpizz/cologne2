#pragma once
#include <engine/core/UUID.h>
#include <engine/physics/Physics.h>
#include <engine/util/Util.h>
#include <entt/entt.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <engine/renderer/types/LightHandle.h>
#include "ScriptableEntity.h"

namespace cologne
{
    struct IDComponent
    {
        UUID id;
    };

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

        bool operator==(const TransformComponent & transform) const = default;
    };

    struct ParentComponent
    {
        std::vector<UUID> children;
    };

    struct ChildComponent
    {
        UUID parent;
    };

    struct RigidbodyComponent
    {
        uint32_t body_id;

        glm::mat4 get_transform()
        {
            return Physics::get_rigidbody_transform(body_id);
        }
    };

    struct ConvexMeshColliderComponent
    {
        std::string mesh_name;
    };

    struct ActiveComponent
    {
        bool active = true;
        explicit operator bool() { return active; }
        explicit operator const bool() const { return active; }
    };

    struct ModelComponent
    {
        std::string model_name;
        bool gi_only = false;
    };

    struct MeshComponent
    {
        MeshComponent() = default;
        MeshComponent(const std::string& name);
        MeshComponent(int idx);

        std::string mesh_name;
    };

    struct SkinnedModelComponent
    {
        std::string model_name;
    };

    struct TagComponent
    {
        std::string tag = std::string();
    };

    struct StaticColliderComponent
    {
        uint32_t body_id = 0;
        std::string mesh_name;
        bool body_enabled = true;
        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

    struct CameraComponent
    {
        float fov_radians = glm::radians(45.0f);
        bool primary = false;
        bool orthographic = false;
        float ortho_zoom = 5.0f;
    };

    struct PlayerComponent
    {
        uint32_t id = 0;
        UUID camera = {};
        UUID viewmodel = {};
        float gravity = 9.8f * 2.0f;
        float move_speed = 5.0f;
        float run_acceleration = 7.0f;
        float run_deceleration = 3.0f;
        float air_acceleration = 2.0f;
        float air_deceleration = 2.0f;
        float air_control = 0.1f;
        float side_strafe_acceleration = 15.0f;
        float side_strafe_speed = 1.0f;
        float jump_speed = 7.0f;
        float friction = 6.0f;
        float maxStepVelocity = 12.5;
        float minStepVelocity = 2.50f;
        float minStepInterval = 0.150f;
        float maxStepInterval = 1.250f;

        void teleport_to_position(glm::vec3 pos)
        {
            cologne::Physics::teleport_player(id, pos);
        }
    };

    struct ViewmodelComponent
    {
        glm::vec3 position_offset = glm::vec3(0.045, -0.270, -0.2);
        glm::vec3 euler_offset = glm::vec3(0.0f, 180.0f, 0.0f);
        float sway_multiplier = 100.0f;
        float smoothing = 8.0f;
        float amplitude = 0.01f;
        float frequency = 12.0f;
        float vertical_velocity_multiplier = 0.01f;
        float max_vertical_offset = 0.07f;
    };

    struct EnemyComponent
    {
        float health = 100.0f;
        bool dead = false;
        std::string hurt_sound = RESOURCES_PATH "sounds/enemy_hurt.mp3";
    };

    struct BulletComponent
    {
        glm::vec3 position;
        glm::vec3 direction;
        float damage;
    };

    struct LightComponent
    {
        enum LightType
        {
            Directional = 0,
            Point = 1,
            Spot = 2
        };

        glm::vec3 color = glm::vec4(1, 0.7799999713897705, 0.5289999842643738, 1.0f);
        float strength = 1.0f;
        float radius = 3.0f;
        int type = LightType::Point;
        float outer_cutoff = 17.5f;
        float inner_cutoff = 12.5f;
        bool cast_shadows = false;
        static void on_construct(entt::registry &registry, const entt::entity entt);
        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };


    struct LightHandleComponent
    {
        LightHandle light_handle;
    };


    struct NativeScriptComponent
    {
        ScriptableEntity *instance = nullptr;

        ScriptableEntity * (*instantiate_script)() = nullptr;

        void (*destroy_script)(NativeScriptComponent *) = nullptr;

        std::string type_name = std::string();

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
