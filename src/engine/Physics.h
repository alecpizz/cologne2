#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "renderer/Model.h"
#include "renderer/Vertex.h"

namespace goon::physics
{
    void init();

    void update(float dt);

    static constexpr uint8_t NON_MOVING(0);
    static constexpr uint8_t MOVING(1);
    //TODO: Get these out!
    JPH::PhysicsSystem* get_physics_system();
    JPH::TempAllocator* get_temp_allocator();
    void create_mesh_collider(Model*, const Vertex *vertices, size_t num_vertices,
               const uint32_t *indices, size_t num_indices);
    void update_mesh_collider(Model* model);

    void destroy();
}
