#pragma once
#include <engine/renderer/types/Model.h>
#include <engine/scene/Components.h>

namespace cologne
{
    struct RaycastHitInfo;
}

namespace cologne::Physics
{
    void init();

    void update(float dt);

    static constexpr uint8_t NON_MOVING(0);
    static constexpr uint8_t MOVING(1);
    uint32_t create_player(PlayerCreateInfo& info);
    void move_player(uint32_t id, PlayerMovementCommand cmd);
    bool player_is_grounded(uint32_t id);
    bool player_is_supported(uint32_t id);
    bool slope_to_steep_for_player(uint32_t id);
    glm::vec3 get_player_position(uint32_t id);
    glm::vec3 get_player_velocity(uint32_t id);
    glm::vec3 get_gravity();
    glm::vec3 get_player_ground_velocity(uint32_t id);
    uint32_t create_static_mesh_collider(Entity entity, TransformComponent transform, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    bool raycast(glm::vec3 origin, glm::vec3 direction, float max_distance, uint32_t layers, RaycastHitInfo& info);
    // void create_static_mesh_collider(Model& model);
    void destroy();
}
