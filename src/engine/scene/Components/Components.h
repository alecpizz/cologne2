#pragma once
#include <engine/animation/Skeleton.h>
#include <engine/animation/SkeletonPose.h>
#include <engine/core/UUID.h>
#include <engine/physics/Physics.h>
#include <engine/util/Util.h>
#include <entt/entt.hpp>
#include <engine/core/Color.h>
#include <engine/renderer/types/LightHandle.h>

namespace cologne
{
    //todo: split into multple files
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

        bool operator==(const TransformComponent &transform) const = default;
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

        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
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

        MeshComponent(const std::string &name);

        MeshComponent(int idx);

        std::string mesh_name;
    };

    struct SkinnedModelComponent
    {
        std::string model_name;

        SkinnedModelComponent(const char *name)
        {
            model_name = name;
        }

        SkinnedModelComponent() = default;

        SkinnedModelComponent(const SkinnedModelComponent &other)
        {
            model_name = other.model_name;
        }

        //runtime
        Skeleton skeleton;
        SkeletonPose skeleton_pose;
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

        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

    struct PlayerControllerComponent
    {
        std::vector<std::string> footstep_sounds;
        glm::vec2 rotation = glm::vec2(0.0f);
        bool is_free_cam = false;
        bool show_mouse = false;
        bool allow_sliding = true;
        bool was_grounded = false;
        bool grounded = true;
        glm::vec3 desired_velocity;
        bool footstep_played = false;
        float bob_time = 0.0f;
        float bob_offset = 0.0f;
        glm::vec3 velocity = glm::vec3(0.0f);
        bool jump_queued = false;
        float step_timer = 0.0f;
        float step_time = .01f;
        float rpm = 60.0f / 600.0f;
        float reload_time = 0.25f;
        float shot_timer = 0.0f;
        int max_ammo = 10;
        int current_ammo = 0;
        float gun_time = 0.0f;
        bool is_firing = false;
        bool is_reloading = false;
        const char *shoot_sound = ASSETS_PATH "sounds/vsk_fire.ogg";
        const char *reload_sound = ASSETS_PATH "sounds/vsk_reload_empty.ogg";
        //view model stuff
        float time = 0.0f;
        TransformComponent prev_transform;
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
        std::string hurt_sound = ASSETS_PATH "sounds/enemy_hurt.mp3";
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
        bool always_update_shadows = true;

        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };


    struct LightHandleComponent
    {
        LightHandle light_handle;

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

    struct HideInEditorComponent
    {
    };

    struct InteractorComponent
    {
        bool update_every_frame = true;

        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

    struct InteractionControllerComponent
    {
        //dunno what can live in here yet :3
        uint32_t last_entity = 0;
        uint32_t current_entity = 0;
    };

    struct EditorCameraComponent
    {
        static void on_construct(entt::registry &registry, const entt::entity entt);

        static void on_destroy(entt::registry &registry, const entt::entity entt);
    };

    struct EditorCameraControllerComponent
    {
        glm::vec2 rotation = glm::vec2(0.0f);
    };

    struct AnimatorComponent
    {
        std::string source_clip_name;
        float source_time = 0.0f;

        std::string dest_clip_name;
        float dest_time = 0.0f;

        float blend_time = 0.0f;
        float blend_duration = 0.2f;

        std::string one_shot_name;
        std::string one_shot_return_clip;

        void crossfade_to(const std::string& name, float duration = 0.2f)
        {
            if (dest_clip_name == name) return;
            if (dest_clip_name.empty() && source_clip_name == name) return;

            // Start a new blend
            dest_clip_name = name;
            dest_time = 0.0f;
            blend_time = 0.0f;
            blend_duration = duration;

            one_shot_name = "";
        }

        void play_one_shot(const std::string &name, float duration = 0.1f)
        {
            if (source_clip_name == name && dest_clip_name.empty())
            {
                source_time = 0.0f;
                return;
            }

            one_shot_return_clip = source_clip_name;
            one_shot_name = name;

            dest_clip_name = name;
            dest_time = 0.0f;
            blend_time = 0.0f;
            blend_duration = duration;
        }
    };

    struct RagdollComponent
    {
        enum class State
        {
            ACTIVE,
            KINEMATIC
        };

        State current_state = State::KINEMATIC;
        uint32_t id = UINT32_MAX;
        std::unordered_map<std::string, uint32_t> bone_to_ragdoll_map = std::unordered_map<std::string, uint32_t>();

        void to_ragdoll();

        void to_kinematic();

        void take_ragdoll_hit(glm::vec3 point, glm::vec3 normal) const;
    };

    struct NPCCrowdMemberComponent
    {
        enum State : int //todo: enum serialization rather than just ints everywhere
        {
            IDLE,
            CHASING,
            ATTACKING,
            DYING
        };

        int agent_id = -1;
        glm::vec3 offset = glm::vec3(0.0f, -0.15f, 0.0f);
        float max_acceleration = 3.5f;
        float max_speed = 1.0f;
        float detection_radius = 20.0f;
        float attack_range = 1.5f;
        float attack_cooldown = 1.2f;
        State current_state = CHASING;
        float state_timer = 0.0f;

        //todo: handles PLEASE
        std::string idle_clip_name;
        std::string run_clip_name;
        std::string attack_clip_name;
        std::string hit_clip_name;
    };



    struct BloodSplatterComponent
    {
        std::string mesh_name;
        std::string position_texture_name;
        std::string normal_texture_name;
        glm::vec3 offset = glm::vec3(0.0f);
        float time = 0.0f;
    };

    struct DecalComponent
    {
        std::string albedo_name;
        std::string normal_name;
        std::string orm_name;
        std::string emission_name;
        Color color_tint = Color(1.0f, 1.0f, 1.0f, 1.0f);
    };
}
