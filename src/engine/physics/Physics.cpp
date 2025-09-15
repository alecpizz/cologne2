//
// Created by alecp on 3/24/2025.
//

#include "Physics.h"

#include <engine/util/DebugScope.h>
#include "RaycastHitInfo.h"
#include <engine/renderer/Renderer.h>
#include <engine/scene/Components.h>
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#include <fstream>
#include "PhysicsUtil.h"
#include "Jolt/Physics/Ragdoll/Ragdoll.h"

JPH_SUPPRESS_WARNINGS

using namespace JPH;
using namespace JPH::literals;

namespace cologne
{
    void TraceImpl(const char *inFMT, ...)
    {
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        // Print to the TTY
        std::cout << buffer << std::endl;
    }



    class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(ObjectLayer inLayer1, ObjectLayer inLayer2) const override
        {
            switch (inLayer1)
            {
                case Layers::NON_MOVING:
                    return inLayer2 == Layers::MOVING || inLayer2 == Layers::PLAYER;
                case Layers::MOVING:
                    return inLayer2 != Layers::PLAYER;
                case Layers::PLAYER:
                    return inLayer2 == Layers::NON_MOVING;
                default:
                    assert(false);
                    return false;
            }
        }
    };

    namespace BroadPhaseLayers
    {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr BroadPhaseLayer PLAYER(2);
        static constexpr uint32_t NUM_LAYERS(3);
    }

    class BroadPhaseLayerImpl final : public BroadPhaseLayerInterface
    {
    public:
        BroadPhaseLayerImpl()
        {
            _object_to_broad_phase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            _object_to_broad_phase[Layers::MOVING] = BroadPhaseLayers::MOVING;
            _object_to_broad_phase[Layers::PLAYER] = BroadPhaseLayers::PLAYER;
        }

        JPH::uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
        {
            assert(inLayer < Layers::NUM_LAYERS);
            return _object_to_broad_phase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char *GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
        {
            switch ((BroadPhaseLayer::Type) inLayer)
            {
                case (BroadPhaseLayer::Type) BroadPhaseLayers::NON_MOVING: return "NON_MOVING";
                case (BroadPhaseLayer::Type) BroadPhaseLayers::MOVING: return "MOVING";
                case (BroadPhaseLayer::Type) BroadPhaseLayers::PLAYER: return "PLAYER";
                default: assert(false);
                    return "INVALID";
            }
        }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

    private:
        BroadPhaseLayer _object_to_broad_phase[Layers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
                case Layers::NON_MOVING:
                    return inLayer2 == BroadPhaseLayers::MOVING;
                case Layers::MOVING:
                    return inLayer2 != BroadPhaseLayers::PLAYER;
                case Layers::PLAYER:
                    return inLayer2 == BroadPhaseLayers::NON_MOVING;
                default:
                    assert(false);
                    return false;
            }
        }
    };



    class BodyDrawFilterImpl final : public BodyDrawFilter
    {
    public:
        bool ShouldDraw(const Body &inBody) const override
        {
            return !inBody.IsStatic();
        }
    };

    TempAllocatorImpl *Physics::_temp_allocator = nullptr;
    JobSystemThreadPool *Physics::_job_system = nullptr;
    BroadPhaseLayerImpl broad_phase_layer;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broad_phase_layer_filter;
    ObjectLayerPairFilterImpl object_vs_object_filter;
    PhysicsSystem Physics::_physics_system;
    BodyDrawFilterImpl body_draw_filter;
    std::vector<JPH::BodyID> Physics::_colliders_static;
    std::unordered_map<JPH::BodyID, Entity> Physics::_entity_to_collider_map;
    std::vector<JPH::Ragdoll *> Physics::_ragdolls;



    void Physics::init()
    {
        LOG_INFO("Initializing Physics");
        DebugScope scope("Physics::init");

        RegisterDefaultAllocator();
        Trace = TraceImpl;
        Factory::sInstance = new Factory();

        JPH::RegisterTypes();

        _temp_allocator = new TempAllocatorImpl(10 * 1024 * 1024);
        _job_system = new JobSystemThreadPool(2048, 8, thread::hardware_concurrency() - 1);
        const uint32_t max_bodies = 65565;
        const uint32_t max_body_mutexes = 0;
        const uint32_t max_body_pairs = 1024;
        const uint32_t max_body_contact_constraints = 1024;
        _physics_system.Init(max_bodies, max_body_mutexes, max_body_pairs, max_body_contact_constraints,
                            broad_phase_layer, object_vs_broad_phase_layer_filter, object_vs_object_filter);

        auto &body_interface = _physics_system.GetBodyInterface();

        _physics_system.OptimizeBroadPhase();
    }

    float accumulation_time = 0.0f;
    float fixed_delta_time = 1.0 / 60.0f;

    void Physics::update(float dt)
    {
        update_players(dt);
        const int collisionSteps = 4;
        accumulation_time += dt;
        while (accumulation_time >= fixed_delta_time)
        {
            _physics_system.Update(fixed_delta_time, collisionSteps, _temp_allocator, _job_system);
            accumulation_time -= fixed_delta_time;
        }
    }

    void Physics::delete_all_bodies()
    {
        _physics_system.GetBodyInterface().RemoveBodies(_colliders_static.data(), _colliders_static.size());
        _physics_system.GetBodyInterface().DestroyBodies(_colliders_static.data(), _colliders_static.size());
        for (auto ragdoll : _ragdolls)
        {
            ragdoll->RemoveFromPhysicsSystem();
            delete ragdoll;
        }
        _entity_to_collider_map.clear();
        cleanup_players();
        _colliders_static.clear();
        _ragdolls.clear();
    }


    void Physics::cleanup()
    {
        delete_all_bodies();
        UnregisterTypes();
        delete _temp_allocator;
        delete _job_system;
        delete Factory::sInstance;
        Factory::sInstance = nullptr;
    }

    void Physics::destroy_body(uint32_t body_id)
    {
        if (_colliders_static.empty())
        {
            return;
        }
        auto jph_body = static_cast<BodyID>(body_id);
        if (jph_body.IsInvalid())
        {
            return;
        }
        disable_body(body_id);
        _physics_system.GetBodyInterface().DestroyBody(static_cast<BodyID>(body_id));
        std::erase(_colliders_static, static_cast<BodyID>(body_id));
    }

    void Physics::add_impulse_force_at_position(uint32_t body_id, glm::vec3 position, glm::vec3 force, bool ignore_mass)
    {
        BodyID id(body_id);
        if (id.IsInvalid())
        {
            return;
        }

        if (ignore_mass)
        {
            force *= _physics_system.GetBodyInterface().GetShape(id)->GetMassProperties().mMass;
        }
        _physics_system.GetBodyInterface().AddImpulse(id, PhysicsUtil::glm_vec3_to_jph_vec3(force), PhysicsUtil::glm_vec3_to_jph_vec3(position));
    }

    void Physics::disable_body(uint32_t body_id)
    {
        _physics_system.GetBodyInterface().RemoveBody(static_cast<BodyID>(body_id));
    }

    void Physics::enable_body(uint32_t body_id)
    {
        _physics_system.GetBodyInterface().AddBody(static_cast<BodyID>(body_id), EActivation::DontActivate);
    }

    void Physics::destroy_entity(Entity entity)
    {
        if (!entity)
        {
            return;
        }
        if (entity.has_component<StaticColliderComponent>())
        {
            auto id = entity.get_component<StaticColliderComponent>().body_id;
            if (id != 0)
            {
                destroy_body(id);
            }
        }

        if (entity.has_component<RigidbodyComponent>())
        {
            auto id = entity.get_component<RigidbodyComponent>().body_id;
            if (id != 0)
            {
                destroy_body(id);
            }
        }
    }
}
