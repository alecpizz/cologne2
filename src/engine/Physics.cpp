//
// Created by alecp on 3/24/2025.
//

#include "Physics.h"
#include "gpch.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/RegisterTypes.h>

#include "Engine.h"
#include "Jolt/Renderer/DebugRendererSimple.h"
#include "renderer/DebugRenderer.h"

JPH_SUPPRESS_WARNINGS

using namespace JPH;
using namespace JPH::literals;


namespace goon::physics
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

    namespace Layers
    {
        static constexpr ObjectLayer NON_MOVING = 0;
        static constexpr ObjectLayer MOVING = 1;
        static constexpr ObjectLayer NUM_LAYERS = 2;
    }

    class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(ObjectLayer inLayer1, ObjectLayer inLayer2) const override
        {
            switch (inLayer1)
            {
                case Layers::NON_MOVING:
                    return inLayer2 == Layers::MOVING;
                case Layers::MOVING:
                    return true;
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
        static constexpr uint32_t NUM_LAYERS(2);
    }

    class BroadPhaseLayerImpl final : public BroadPhaseLayerInterface
    {
    public:
        BroadPhaseLayerImpl()
        {
            _object_to_broad_phase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            _object_to_broad_phase[Layers::MOVING] = BroadPhaseLayers::MOVING;
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
                    return true;
                default:
                    assert(false);
                    return false;
            }
        }
    };

    class PhysDebugRenderer : public JPH::DebugRendererSimple
    {
    public:
        void DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override
        {
            Engine::get_debug_renderer()->draw_line(glm::vec3(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()),
                glm::vec3(inTo.GetX(), inTo.GetY(), inTo.GetZ()),
                glm::vec3(inColor.r, inColor.g, inColor.b));
        }
        void DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow) override
        {
            Engine::get_debug_renderer()->draw_triangle(glm::vec3(inV1.GetX(), inV1.GetY(), inV1.GetZ()),
                glm::vec3(inV2.GetX(), inV2.GetY(), inV2.GetZ()),
                glm::vec3(inV3.GetX(), inV3.GetY(), inV3.GetZ()),
                glm::vec3(inColor.r, inColor.g, inColor.b));
        }
        void DrawText3D(RVec3Arg inPosition, const string_view &inString, ColorArg inColor, float inHeight)
        {
            //TODO!
        }
    };

    TempAllocatorImpl* temp_allocator = nullptr;
    JobSystemThreadPool* job_system = nullptr;
    BroadPhaseLayerImpl broad_phase_layer;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broad_phase_layer_filter;
    ObjectLayerPairFilterImpl object_vs_object_filter;
    PhysicsSystem physics_system;
    Body *floor = nullptr;
    BodyID box_id;
    PhysDebugRenderer* debug_renderer = nullptr;

    void init()
    {
        LOG_INFO("Initializing Physics");

        RegisterDefaultAllocator();
        Trace = TraceImpl;
        Factory::sInstance = new Factory();

        JPH::RegisterTypes();

        temp_allocator = new TempAllocatorImpl(10 * 1024 * 1024);
        job_system = new JobSystemThreadPool(2048, 8, thread::hardware_concurrency() - 1);
        const uint32_t max_bodies = 1024;
        const uint32_t max_body_mutexes = 0;
        const uint32_t max_body_pairs = 1024;
        const uint32_t max_body_contact_constraints = 1024;
        physics_system.Init(max_bodies, max_body_mutexes, max_body_pairs, max_body_contact_constraints,
                            broad_phase_layer, object_vs_broad_phase_layer_filter, object_vs_object_filter);

        auto &body_interface = physics_system.GetBodyInterface();
        debug_renderer = new PhysDebugRenderer();
        //make a floor
        BoxShapeSettings floor_shape_settings = (Vec3(100.0f, 1.0f, 100.0f));
        floor_shape_settings.SetEmbedded();

        ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
        ShapeRefC floor_shape = floor_shape_result.Get();

        BodyCreationSettings floor_settings(floor_shape,
                                            RVec3(0.0f, -1.0f, 0.0f), Quat::sIdentity(),
                                            EMotionType::Static, Layers::NON_MOVING);
        floor = body_interface.CreateBody(floor_settings);

        body_interface.AddBody(floor->GetID(), EActivation::DontActivate);

        BodyCreationSettings box_settings(new BoxShape(Vec3(1.0f, 1.0f, 1.0f)),
                                          RVec3(0.0f, 2.0f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic,
                                          Layers::MOVING);
        box_id = body_interface.CreateAndAddBody(box_settings, EActivation::Activate);

        physics_system.OptimizeBroadPhase();
    }

    void update(float dt)
    {
        const int collisionSteps = 1;
        physics_system.Update(dt, collisionSteps, temp_allocator, job_system);
        BodyManager::DrawSettings draw_settings;
        draw_settings.mDrawShape = true;
        draw_settings.mDrawShapeWireframe = true;
        physics_system.DrawBodies(draw_settings, debug_renderer);
    }

    void destroy()
    {
        auto& body_interface = physics_system.GetBodyInterface();
        body_interface.RemoveBody(box_id);
        body_interface.DestroyBody(box_id);
        body_interface.RemoveBody(floor->GetID());
        body_interface.DestroyBody(floor->GetID());
        UnregisterTypes();
        delete temp_allocator;
        delete debug_renderer;
        delete job_system;
        delete Factory::sInstance;
        Factory::sInstance = nullptr;
    }
}
