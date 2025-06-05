//
// Created by alecp on 3/24/2025.
//

#include "Physics.h"

#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include <engine/scene/Components.h>
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Renderer/DebugRendererSimple.h>

#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>


namespace JPH
{
    class CharacterVirtualSettings;
    class CharacterVirtual;
}

JPH_SUPPRESS_WARNINGS

using namespace JPH;
using namespace JPH::literals;


namespace cologne::Physics
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
            Engine::get_renderer()->draw_line(glm::vec3(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()),
                                              glm::vec3(inTo.GetX(), inTo.GetY(), inTo.GetZ()),
                                              glm::vec3(inColor.r, inColor.g, inColor.b));
        }

        void DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor,
                          ECastShadow inCastShadow) override
        {
            Engine::get_renderer()->draw_triangle(glm::vec3(inV1.GetX(), inV1.GetY(), inV1.GetZ()),
                                                  glm::vec3(inV2.GetX(), inV2.GetY(), inV2.GetZ()),
                                                  glm::vec3(inV3.GetX(), inV3.GetY(), inV3.GetZ()),
                                                  glm::vec3(inColor.r, inColor.g, inColor.b));
        }

        void DrawText3D(RVec3Arg inPosition, const string_view &inString, ColorArg inColor, float inHeight)
        {
            //TODO!
        }
    };

    struct PhysicsPlayer
    {
        JPH::RefConst<JPH::Shape> standing_shape;
        JPH::RefConst<JPH::Shape> crouching_shape;
        JPH::RefConst<JPH::Shape> inner_standing_shape;
        JPH::RefConst<JPH::Shape> inner_crouching_shape;
        JPH::Ref<JPH::CharacterVirtual> character;
        glm::vec3 character_position;
    };

    TempAllocatorImpl *temp_allocator = nullptr;
    JobSystemThreadPool *job_system = nullptr;
    BroadPhaseLayerImpl broad_phase_layer;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broad_phase_layer_filter;
    ObjectLayerPairFilterImpl object_vs_object_filter;
    PhysicsSystem physics_system;
    PhysDebugRenderer *debug_renderer = nullptr;
    std::vector<JPH::BodyID> colliders_static;
    std::unordered_map<JPH::BodyID, Entity> entity_to_collider_map;
    std::unordered_map<uint32_t, PhysicsPlayer> physics_players;
    bool drawing = false;


    void init()
    {
        LOG_INFO("Initializing Physics");

        RegisterDefaultAllocator();
        Trace = TraceImpl;
        Factory::sInstance = new Factory();

        JPH::RegisterTypes();

        temp_allocator = new TempAllocatorImpl(10 * 1024 * 1024);
        job_system = new JobSystemThreadPool(2048, 8, thread::hardware_concurrency() - 1);
        const uint32_t max_bodies = 65565;
        const uint32_t max_body_mutexes = 0;
        const uint32_t max_body_pairs = 1024;
        const uint32_t max_body_contact_constraints = 1024;
        physics_system.Init(max_bodies, max_body_mutexes, max_body_pairs, max_body_contact_constraints,
                            broad_phase_layer, object_vs_broad_phase_layer_filter, object_vs_object_filter);

        auto &body_interface = physics_system.GetBodyInterface();
        debug_renderer = new PhysDebugRenderer();

        physics_system.OptimizeBroadPhase();
    }

    void update(float dt)
    {
        for (auto &physics_player: physics_players)
        {
            auto &p = physics_player.second;
            auto &character = p.character;
            JPH::CharacterVirtual::ExtendedUpdateSettings update_settings;
            update_settings.mStickToFloorStepDown = -character->GetUp() * update_settings.mStickToFloorStepDown.
                                                    Length();
            update_settings.mWalkStairsStepUp = character->GetUp() * update_settings.mWalkStairsStepUp.Length();
            character->ExtendedUpdate(
                dt, character->GetUp() * physics_system.GetGravity().Length(), update_settings,
                physics_system.GetDefaultBroadPhaseLayerFilter(1),
                physics_system.GetDefaultLayerFilter(1),
                {},
                {},
                *temp_allocator);

            p.character_position = glm::vec3(character->GetPosition().GetX(), character->GetPosition().GetY(),
                                             character->GetPosition().GetZ());
        }

        if (cologne::Input::key_pressed(Input::Key::P))
        {
            drawing = !drawing;
        }
        const int collisionSteps = 1;
        physics_system.Update(dt, collisionSteps, temp_allocator, job_system);
        if (drawing)
        {
            BodyManager::DrawSettings draw_settings;
            draw_settings.mDrawShape = true;
            draw_settings.mDrawShapeWireframe = true;
            physics_system.DrawBodies(draw_settings, debug_renderer);
        }
    }

    JPH::PhysicsSystem *get_physics_system()
    {
        return &physics_system;
    }

    JPH::TempAllocator *get_temp_allocator()
    {
        return temp_allocator;
    }

    JPH::Vec3 glm_vec3_to_vec3(const glm::vec3 &v)
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }

    JPH::Quat glm_quat_to_jph_quat(const glm::quat& q)
    {
        return JPH::Quat(q.x, q.y, q.z, q.w).Normalized();
    }

    glm::vec3 jph_vec3_to_glm_vec3(const JPH::Vec3& v)
    {
        return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
    }

    uint32_t create_player(PlayerCreateInfo &info)
    {
        PhysicsPlayer player;
        player.standing_shape = JPH::RotatedTranslatedShapeSettings(
            JPH::Vec3(0, 0.5f * info.height_standing + info.radius_standing, 0), JPH::Quat::sIdentity(),
            new JPH::CapsuleShape(0.5f * info.height_standing, info.radius_standing)).Create().Get();
        player.inner_standing_shape = JPH::RotatedTranslatedShapeSettings(
            JPH::Vec3(0, 0.5f * info.height_standing + info.radius_standing, 0), JPH::Quat::sIdentity(),
            new JPH::CapsuleShape(0.5f * info.inner_friction * info.height_standing,
                                  info.inner_friction * info.radius_standing)).Create().Get();

        JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
        settings->mMaxSlopeAngle = glm::radians(45.0f);
        settings->mMaxStrength = 100.0f;
        settings->mShape = player.standing_shape;
        settings->mBackFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;
        settings->mCharacterPadding = 0.02f;
        settings->mPenetrationRecoverySpeed = 1.0f;
        settings->mPredictiveContactDistance = 0.1f;
        settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -info.radius_standing);
        settings->mEnhancedInternalEdgeRemoval = false;
        settings->mInnerBodyShape = player.inner_standing_shape;
        settings->mInnerBodyLayer = cologne::Physics::NON_MOVING;


        player.character = new JPH::CharacterVirtual(settings, glm_vec3_to_vec3(info.position),
                                                     JPH::Quat::sIdentity(), 0, &physics_system);
        uint32_t id = player.character->GetID().GetValue();
        physics_players[id] = player;
        return id;
    }

    glm::vec3 get_player_position(uint32_t id)
    {
        if (!physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return glm::vec3(0.0f);
        }
        return physics_players[id].character_position;
    }

    void move_player(uint32_t id, PlayerMovementCommand cmd)
    {
        if (!physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return;
        }        auto& character = physics_players[id].character;
        character->SetUp(glm_vec3_to_vec3(cmd.up));
        character->SetRotation(glm_quat_to_jph_quat(cmd.rotation));
        character->SetLinearVelocity(glm_vec3_to_vec3(cmd.movement));
    }

    bool player_is_grounded(uint32_t id)
    {
        if (!physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return false;
        }
        return physics_players[id].character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround;
    }

    bool player_is_supported(uint32_t id)
    {
        if (!physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return false;
        }
        return physics_players[id].character->IsSupported();
    }

    bool slope_to_steep_for_player(uint32_t id)
    {
        if (!physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return false;
        }
        return physics_players[id].character->IsSlopeTooSteep(physics_players[id].character->GetGroundNormal());
    }

    glm::vec3 get_gravity()
    {
        return jph_vec3_to_glm_vec3(physics_system.GetGravity());
    }


    glm::vec3 get_player_velocity(uint32_t id)
    {
        if (!physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return glm::vec3(0.0F);
        }
        return jph_vec3_to_glm_vec3(physics_players[id].character->GetLinearVelocity());
    }

    glm::vec3 get_player_ground_velocity(uint32_t id)
    {
        if (!physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return glm::vec3(0.0F);
        }
        return jph_vec3_to_glm_vec3(physics_players[id].character->GetGroundVelocity());
    }


    JPH::Float3 glm_vec3_to_float3(const glm::vec3 &v)
    {
        return JPH::Float3(v.x, v.y, v.z);
    }

    JPH::Vec3 glm_vec3_to_jph_vec3(const glm::vec3 &v)
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }

    uint32_t create_static_mesh_collider(Entity entity, TransformComponent transform,
                                         const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    {
        JPH::TriangleList triangle_list;
        for (int i = 0; i * 3 < indices.size(); i++)
        {
            Triangle triangle =
            {
                glm_vec3_to_float3(vertices[indices[3 * i]].position),
                glm_vec3_to_float3(vertices[indices[3 * i + 1]].position),
                glm_vec3_to_float3(vertices[indices[3 * i + 2]].position)
            };
            triangle_list.emplace_back(triangle);
        }
        JPH::MeshShapeSettings mesh_settings(triangle_list);
        mesh_settings.SetEmbedded();
        JPH::BodyCreationSettings settings(&mesh_settings, JPH::Vec3::sZero(),
                                           JPH::Quat::sIdentity(), JPH::EMotionType::Static,
                                           cologne::Physics::NON_MOVING);
        auto &body_interface = physics_system.GetBodyInterface();
        auto id = body_interface.CreateAndAddBody(
            settings, JPH::EActivation::DontActivate);
        const auto shape = body_interface.GetShape(id);
        const auto new_shape = shape->ScaleShape(glm_vec3_to_jph_vec3(transform.scale)).Get();
        body_interface.SetShape(id, new_shape, true, EActivation::DontActivate);
        body_interface.SetPositionAndRotation(id, glm_vec3_to_jph_vec3(transform.position),
                                              glm_quat_to_jph_quat(transform.rotation),
                                              EActivation::DontActivate);
        LOG_INFO("Created id %d", id);
        colliders_static.push_back(id);
        physics_system.OptimizeBroadPhase();
        entity_to_collider_map[id] = entity;
        return id.GetIndexAndSequenceNumber();
    }

    // void create_static_mesh_collider(Model &model)
    // {
    //     const Transform tr = model.get_transform();
    //     LOG_INFO("CREATING MESH COLLDIER WITH SCALE %f %f %f", tr.scale.x, tr.scale.y, tr.scale.z);
    //     for (size_t i = 0; i < model.get_num_meshes(); i++)
    //     {
    //         auto& mesh = model.get_meshes()[i];
    //         auto vertices = mesh.get_vertices();
    //         auto indices = mesh.get_indices();
    //         Physics::create_static_mesh_collider(tr, vertices, indices);
    //     }
    // }

    void destroy()
    {
        physics_system.GetBodyInterface().RemoveBodies(colliders_static.data(), colliders_static.size());
        physics_system.GetBodyInterface().DestroyBodies(colliders_static.data(), colliders_static.size());
        UnregisterTypes();
        delete temp_allocator;
        delete debug_renderer;
        delete job_system;
        delete Factory::sInstance;
        Factory::sInstance = nullptr;
    }
}
