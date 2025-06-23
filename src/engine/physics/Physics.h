#pragma once
#include <engine/Types.h>

namespace cologne
{
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
    uint32_t create_player(PlayerCreateInfo& info);
    void move_player(uint32_t id, PlayerMovementCommand cmd);
    void teleport_player(uint32_t id, glm::vec3 position);
    bool player_is_grounded(uint32_t id);
    bool player_is_supported(uint32_t id);
    bool slope_to_steep_for_player(uint32_t id);
    glm::vec3 get_player_position(uint32_t id);
    glm::vec3 get_player_velocity(uint32_t id);
    glm::vec3 get_gravity();
    glm::vec3 get_player_ground_velocity(uint32_t id);
    uint32_t create_static_mesh_collider(Entity entity, TransformComponent transform, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    void sync_transform(Entity entity);
    bool raycast(glm::vec3 origin, glm::vec3 direction, float max_distance, uint32_t layers, RaycastHitInfo& info);
    // void create_static_mesh_collider(Model& model);
    void cleanup();
    void destroy_body(uint32_t body_id);
    void disable_body(uint32_t body_id);
    void enable_body(uint32_t body_id);
}
