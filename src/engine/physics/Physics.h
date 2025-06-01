#pragma once
#include <engine/renderer/types/Model.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>



namespace cologne::Physics
{
    void init();

    void update(float dt);

    static constexpr uint8_t NON_MOVING(0);
    static constexpr uint8_t MOVING(1);
    //TODO: Get these out!
    JPH::PhysicsSystem* get_physics_system();
    JPH::TempAllocator* get_temp_allocator();
    void create_mesh_collider(Transform transform, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    void update_mesh_collider(Model* model);

    void destroy();
}
