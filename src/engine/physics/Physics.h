#pragma once

namespace JPH
{
    class CharacterVirtual;
    class Shape;
    class Ragdoll;
    class BodyID;
    class PhysicsSystem;
    class JobSystemThreadPool;
    class TempAllocatorImpl;
}


namespace cologne
{
    class Skeleton;
    struct PlayerMovementCommand;
    struct PlayerCreateInfo;
    struct PhysicsPlayer;
    class Mesh;
    struct RaycastHitInfo;
    struct TransformComponent;
    class Entity;
}

namespace Layers
{
    static constexpr uint32_t NON_MOVING = 0;
    static constexpr uint32_t MOVING = 1;
    static constexpr uint32_t PLAYER = 2;
    static constexpr uint32_t NUM_LAYERS = 3;
}

namespace cologne
{
    class Physics
    {
    public:
        static void init();

        static void update(float dt);

        static void draw();

        static constexpr uint8_t NON_MOVING = 0;

        static constexpr uint8_t MOVING = 1;

        static constexpr uint8_t PLAYER = 2;

        static uint32_t create_player(PlayerCreateInfo &info);

        static void move_player(uint32_t id, PlayerMovementCommand cmd);

        static void teleport_player(uint32_t id, glm::vec3 position);

        static bool player_is_grounded(uint32_t id);

        static bool player_is_supported(uint32_t id);

        static bool slope_to_steep_for_player(uint32_t id);

        static glm::vec3 get_player_position(uint32_t id);

        static glm::vec3 get_player_velocity(uint32_t id);

        static glm::vec3 get_gravity();

        static glm::vec3 get_player_ground_velocity(uint32_t id);

        static uint32_t create_static_mesh_collider(Entity entity, TransformComponent transform,
                                                    const Mesh &mesh);

        static uint32_t create_infinite_ground_plane(glm::vec3 plane_normal, float constant);

        static void sync_transform(Entity entity);

        static bool raycast(glm::vec3 origin, glm::vec3 direction, float max_distance, uint32_t layers,
                            RaycastHitInfo &info);

        // void create_static_mesh_collider(Model& model);
        static void delete_all_bodies();

        static void cleanup();

        static void destroy_body(uint32_t body_id);

        static void add_impulse_force_at_position(uint32_t body_id, glm::vec3 position, glm::vec3 force,
                                                  bool ignore_mass = false);

        static void disable_body(uint32_t body_id);

        static void enable_body(uint32_t body_id);

        static uint32_t create_ragdoll(Entity entity, std::unordered_map<std::string, uint32_t> &out_map,
                                       const Skeleton &skeleton, const std::vector<glm::mat4> &global_bind_transforms);

        static void make_ragdoll_kinematic(uint32_t ragdoll_id);

        static void make_ragdoll_active(uint32_t ragdoll_id);

        static void sync_ragdoll(uint32_t ragdoll_id,
                                 const std::unordered_map<std::string, glm::mat4> &ragdoll_transforms);

        static glm::mat4 get_rigidbody_transform(uint32_t body_id);

        static uint32_t create_rigidbody(Entity entity);

        static void destroy_entity(Entity entity);

    private:
        static void update_players(float dt);
        static void cleanup_players();

        static JPH::TempAllocatorImpl *_temp_allocator;
        static JPH::JobSystemThreadPool *_job_system;
        static JPH::PhysicsSystem _physics_system;
        static std::vector<JPH::BodyID> _colliders_static;
        static std::unordered_map<JPH::BodyID, Entity> _entity_to_collider_map;
        static std::unordered_map<uint32_t, PhysicsPlayer> _physics_players;
        static std::vector<JPH::Ragdoll *> _ragdolls;

        static bool _drawing;
    };
}
