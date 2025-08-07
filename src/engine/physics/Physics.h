#pragma once
#include <engine/Types.h>
#include "RagdollCreateInfo.h"

namespace cologne
{
    class Mesh;
    struct RaycastHitInfo;
    struct TransformComponent;
    class Entity;
}

namespace cologne::Physics
{
    void init();

    void update(float dt);

    static constexpr uint8_t NON_MOVING(0);
    static constexpr uint8_t MOVING(1);
    static constexpr uint8_t PLAYER(2);

    uint32_t create_player(PlayerCreateInfo &info);

    void move_player(uint32_t id, PlayerMovementCommand cmd);

    void teleport_player(uint32_t id, glm::vec3 position);

    bool player_is_grounded(uint32_t id);

    bool player_is_supported(uint32_t id);

    bool slope_to_steep_for_player(uint32_t id);

    glm::vec3 get_player_position(uint32_t id);

    glm::vec3 get_player_velocity(uint32_t id);

    glm::vec3 get_gravity();

    glm::vec3 get_player_ground_velocity(uint32_t id);

    uint32_t create_static_mesh_collider(Entity entity, TransformComponent transform,
                                         const Mesh& mesh);

    uint32_t create_infinite_ground_plane(glm::vec3 plane_normal, float constant);

    void sync_transform(Entity entity);

    bool raycast(glm::vec3 origin, glm::vec3 direction, float max_distance, uint32_t layers, RaycastHitInfo &info);

    // void create_static_mesh_collider(Model& model);
    void delete_all_bodies();

    void cleanup();

    void destroy_body(uint32_t body_id);

    void add_impulse_force_at_position(uint32_t body_id, glm::vec3 position, glm::vec3 force);

    void disable_body(uint32_t body_id);

    void enable_body(uint32_t body_id);

    uint32_t create_ragdoll(Entity entity, std::unordered_map<std::string, uint32_t> &out_map, const Skeleton& skeleton, const std::vector<glm::mat4>& global_bind_transforms);
    void make_ragdoll_kinematic(uint32_t ragdoll_id);
    void make_ragdoll_active(uint32_t ragdoll_id);
    void sync_ragdoll(uint32_t ragdoll_id, const std::unordered_map<std::string, glm::mat4>& ragdoll_transforms);
    glm::mat4 get_rigidbody_transform(uint32_t body_id);

    uint32_t create_rigidbody(Entity entity);
}
