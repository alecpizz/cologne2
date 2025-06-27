//
// Created by alecp on 3/24/2025.
//

#include "Physics.h"

#include <engine/util/DebugScope.h>
#include "RaycastHitInfo.h"
#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer.h>
#include <engine/scene/Components.h>
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>

#include "Jolt/Physics/Constraints/SwingTwistConstraint.h"

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
        static constexpr ObjectLayer PLAYER = 2;
        static constexpr ObjectLayer NUM_LAYERS = 3;
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
                    return true;
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
                case Layers::PLAYER:
                    return true;
                default:
                    assert(false);
                    return false;
            }
        }
    };

    class RayCastLayerFilter final : public ObjectLayerFilter
    {
    private:
        std::vector<uint32_t> _layers;

    public:
        explicit RayCastLayerFilter( const std::vector<uint32_t>& layers = std::vector<uint32_t>())
        {
            _layers = layers;
        }

        ~RayCastLayerFilter() override
        {
        }

        bool ShouldCollide(ObjectLayer inLayer) const override
        {
            if (_layers.empty())
            {
                return true;
            }
            if (std::ranges::find(_layers, inLayer) != _layers.end())
            {
                return true;
            }
            return false;
        }
    };

    class BodyDrawFilterImpl final : public BodyDrawFilter
    {
    public:
        bool ShouldDraw(const Body &inBody) const override
        {
            return inBody.IsActive();
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

    RagdollSettings *create_ragdoll_settings(glm::vec3 position)
    {
        // Create skeleton
        Vec3 origin = {position.x, position.y, position.z};
        Ref<Skeleton> skeleton = new Skeleton;
        uint lower_body = skeleton->AddJoint("LowerBody");
        uint mid_body = skeleton->AddJoint("MidBody", lower_body);
        uint upper_body = skeleton->AddJoint("UpperBody", mid_body);
        /*uint head =*/
        skeleton->AddJoint("Head", upper_body);
        uint upper_arm_l = skeleton->AddJoint("UpperArmL", upper_body);
        uint upper_arm_r = skeleton->AddJoint("UpperArmR", upper_body);
        /*uint lower_arm_l =*/
        skeleton->AddJoint("LowerArmL", upper_arm_l);
        /*uint lower_arm_r =*/
        skeleton->AddJoint("LowerArmR", upper_arm_r);
        uint upper_leg_l = skeleton->AddJoint("UpperLegL", lower_body);
        uint upper_leg_r = skeleton->AddJoint("UpperLegR", lower_body);
        /*uint lower_leg_l =*/
        skeleton->AddJoint("LowerLegL", upper_leg_l);
        /*uint lower_leg_r =*/
        skeleton->AddJoint("LowerLegR", upper_leg_r);

        // Create shapes for limbs
        Ref<Shape> shapes[] = {
            new CapsuleShape(0.15f, 0.10f), // Lower Body
            new CapsuleShape(0.15f, 0.10f), // Mid Body
            new CapsuleShape(0.15f, 0.10f), // Upper Body
            new CapsuleShape(0.075f, 0.10f), // Head
            new CapsuleShape(0.15f, 0.06f), // Upper Arm L
            new CapsuleShape(0.15f, 0.06f), // Upper Arm R
            new CapsuleShape(0.15f, 0.05f), // Lower Arm L
            new CapsuleShape(0.15f, 0.05f), // Lower Arm R
            new CapsuleShape(0.2f, 0.075f), // Upper Leg L
            new CapsuleShape(0.2f, 0.075f), // Upper Leg R
            new CapsuleShape(0.2f, 0.06f), // Lower Leg L
            new CapsuleShape(0.2f, 0.06f), // Lower Leg R
        };

        // Positions of body parts in world space
        RVec3 positions[] = {
            RVec3(0, 1.15f, 0) + origin, // Lower Body
            RVec3(0, 1.35f, 0) + origin, // Mid Body
            RVec3(0, 1.55f, 0) + origin, // Upper Body
            RVec3(0, 1.825f, 0) + origin, // Head
            RVec3(-0.425f, 1.55f, 0) + origin, // Upper Arm L
            RVec3(0.425f, 1.55f, 0) + origin, // Upper Arm R
            RVec3(-0.8f, 1.55f, 0) + origin, // Lower Arm L
            RVec3(0.8f, 1.55f, 0) + origin, // Lower Arm R
            RVec3(-0.15f, 0.8f, 0) + origin, // Upper Leg L
            RVec3(0.15f, 0.8f, 0) + origin, // Upper Leg R
            RVec3(-0.15f, 0.3f, 0) + origin, // Lower Leg L
            RVec3(0.15f, 0.3f, 0) + origin, // Lower Leg R
        };

        // Rotations of body parts in world space
        Quat rotations[] = {
            Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), // Lower Body
            Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), // Mid Body
            Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), // Upper Body
            Quat::sIdentity(), // Head
            Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), // Upper Arm L
            Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), // Upper Arm R
            Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), // Lower Arm L
            Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), // Lower Arm R
            Quat::sIdentity(), // Upper Leg L
            Quat::sIdentity(), // Upper Leg R
            Quat::sIdentity(), // Lower Leg L
            Quat::sIdentity() // Lower Leg R
        };

        // World space constraint positions
        RVec3 constraint_positions[] = {
            RVec3::sZero() + origin, // Lower Body (unused, there's no parent)
            RVec3(0, 1.25f, 0) + origin, // Mid Body
            RVec3(0, 1.45f, 0) + origin, // Upper Body
            RVec3(0, 1.65f, 0) + origin, // Head
            RVec3(-0.225f, 1.55f, 0) + origin, // Upper Arm L
            RVec3(0.225f, 1.55f, 0) + origin, // Upper Arm R
            RVec3(-0.65f, 1.55f, 0) + origin, // Lower Arm L
            RVec3(0.65f, 1.55f, 0) + origin, // Lower Arm R
            RVec3(-0.15f, 1.05f, 0) + origin, // Upper Leg L
            RVec3(0.15f, 1.05f, 0) + origin, // Upper Leg R
            RVec3(-0.15f, 0.55f, 0) + origin, // Lower Leg L
            RVec3(0.15f, 0.55f, 0) + origin, // Lower Leg R
        };

        // World space twist axis directions
        Vec3 twist_axis[] = {
            Vec3::sZero(), // Lower Body (unused, there's no parent)
            Vec3::sAxisY(), // Mid Body
            Vec3::sAxisY(), // Upper Body
            Vec3::sAxisY(), // Head
            -Vec3::sAxisX(), // Upper Arm L
            Vec3::sAxisX(), // Upper Arm R
            -Vec3::sAxisX(), // Lower Arm L
            Vec3::sAxisX(), // Lower Arm R
            -Vec3::sAxisY(), // Upper Leg L
            -Vec3::sAxisY(), // Upper Leg R
            -Vec3::sAxisY(), // Lower Leg L
            -Vec3::sAxisY(), // Lower Leg R
        };

        // Constraint limits
        float twist_angle[] = {
            0.0f, // Lower Body (unused, there's no parent)
            5.0f, // Mid Body
            5.0f, // Upper Body
            90.0f, // Head
            45.0f, // Upper Arm L
            45.0f, // Upper Arm R
            45.0f, // Lower Arm L
            45.0f, // Lower Arm R
            45.0f, // Upper Leg L
            45.0f, // Upper Leg R
            45.0f, // Lower Leg L
            45.0f, // Lower Leg R
        };

        float normal_angle[] = {
            0.0f, // Lower Body (unused, there's no parent)
            10.0f, // Mid Body
            10.0f, // Upper Body
            45.0f, // Head
            90.0f, // Upper Arm L
            90.0f, // Upper Arm R
            0.0f, // Lower Arm L
            0.0f, // Lower Arm R
            45.0f, // Upper Leg L
            45.0f, // Upper Leg R
            0.0f, // Lower Leg L
            0.0f, // Lower Leg R
        };

        float plane_angle[] = {
            0.0f, // Lower Body (unused, there's no parent)
            10.0f, // Mid Body
            10.0f, // Upper Body
            45.0f, // Head
            45.0f, // Upper Arm L
            45.0f, // Upper Arm R
            90.0f, // Lower Arm L
            90.0f, // Lower Arm R
            45.0f, // Upper Leg L
            45.0f, // Upper Leg R
            60.0f, // Lower Leg L (cheating here, a knee is not symmetric, we should have rotated the twist axis)
            60.0f, // Lower Leg R
        };

        // Create ragdoll settings
        auto settings = new RagdollSettings;
        settings->mSkeleton = skeleton;
        settings->mParts.resize(skeleton->GetJointCount());
        for (int p = 0; p < skeleton->GetJointCount(); ++p)
        {
            RagdollSettings::Part &part = settings->mParts[p];
            part.SetShape(shapes[p]);
            part.mPosition = positions[p];
            part.mRotation = rotations[p];
            part.mMotionType = EMotionType::Dynamic;
            part.mObjectLayer = Layers::MOVING;

            // First part is the root, doesn't have a parent and doesn't have a constraint
            if (p > 0)
            {
                auto constraint = new SwingTwistConstraintSettings;
                constraint->mDrawConstraintSize = 0.1f;
                constraint->mPosition1 = constraint->mPosition2 = constraint_positions[p];
                constraint->mTwistAxis1 = constraint->mTwistAxis2 = twist_axis[p];
                constraint->mPlaneAxis1 = constraint->mPlaneAxis2 = Vec3::sAxisZ();
                constraint->mTwistMinAngle = -DegreesToRadians(twist_angle[p]);
                constraint->mTwistMaxAngle = DegreesToRadians(twist_angle[p]);
                constraint->mNormalHalfConeAngle = DegreesToRadians(normal_angle[p]);
                constraint->mPlaneHalfConeAngle = DegreesToRadians(plane_angle[p]);
                part.mToParent = constraint;
            }
        }

        // Optional: Stabilize the inertia of the limbs
        settings->Stabilize();

        // Disable parent child collisions so that we don't get collisions between constrained bodies
        settings->DisableParentChildCollisions();

        // Calculate the map needed for GetBodyIndexToConstraintIndex()
        settings->CalculateBodyIndexToConstraintIndex();

        return settings;
    }


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
    std::vector<JPH::Ragdoll*> ragdolls;
    std::unordered_map<JPH::BodyID, Entity> entity_to_collider_map;
    std::unordered_map<uint32_t, PhysicsPlayer> physics_players;
    BodyDrawFilterImpl body_draw_filter;
    bool drawing = false;
    Ragdoll *ragdoll = nullptr;
    float accumulation_time = 0.0f;

    Ragdoll* create_ragdoll(glm::vec3 position)
    {
        Ref settings = create_ragdoll_settings(position);
        auto ragdoll = settings->CreateRagdoll(0, 0, &physics_system);
        ragdoll->AddToPhysicsSystem(EActivation::Activate);
        ragdolls.emplace_back(ragdoll);
        return ragdoll;
    }

    JPH::BodyInterface & get_body_interface()
    {
        return physics_system.GetBodyInterface();
    }


    void init()
    {
        LOG_INFO("Initializing Physics");
        DebugScope scope("Physics::init");

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
        if (cologne::Input::key_pressed(Input::Key::P))
        {
            drawing = !drawing;
        }
        if (!Engine::in_edit_mode())
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
            const int collisionSteps = 1;
            accumulation_time += dt;
            constexpr float fixed_delta_time = 1.0 / 60;
            //physics updates
            while (accumulation_time >= fixed_delta_time)
            {
                physics_system.Update(fixed_delta_time, collisionSteps, temp_allocator, job_system);
                accumulation_time -= fixed_delta_time;
            }
        }

        if (drawing)
        {
            BodyManager::DrawSettings draw_settings;
            draw_settings.mDrawShape = true;
            draw_settings.mDrawShapeWireframe = true;
            physics_system.DrawBodies(draw_settings, debug_renderer, &body_draw_filter);
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

    JPH::Quat glm_quat_to_jph_quat(const glm::quat &q)
    {
        return JPH::Quat(q.x, q.y, q.z, q.w).Normalized();
    }

    glm::vec3 jph_vec3_to_glm_vec3(const JPH::Vec3 &v)
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
        settings->mInnerBodyLayer = cologne::Physics::PLAYER;


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
        }
        auto &character = physics_players[id].character;
        character->SetUp(glm_vec3_to_vec3(cmd.up));
        character->SetRotation(glm_quat_to_jph_quat(cmd.rotation));
        character->SetLinearVelocity(glm_vec3_to_vec3(cmd.movement));
    }

    void teleport_player(uint32_t id, glm::vec3 position)
    {
        if (!physics_players.contains(id))
        {
            return;
        }

        auto &player = physics_players[id];
        auto &character = player.character;
        character->SetPosition(glm_vec3_to_vec3(position));
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
        auto quat = glm_quat_to_jph_quat(transform.rotation);
        if (!quat.IsNormalized())
        {
            LOG_INFO("Quat isn't normalized!");
            quat = quat.sIdentity();
        }
        body_interface.SetPositionAndRotation(id, glm_vec3_to_jph_vec3(transform.position),
                                              quat,
                                              EActivation::DontActivate);
        LOG_INFO("Created collider with id %d", id);
        colliders_static.push_back(id);
        physics_system.OptimizeBroadPhase();
        entity_to_collider_map[id] = entity;
        return id.GetIndexAndSequenceNumber();
    }

    void sync_transform(Entity entity)
    {
        if (entity.has_component<StaticColliderComponent>())
        {
            auto &comp = entity.get_component<StaticColliderComponent>();
            auto tr = entity.get_component<WorldTransformComponent>().transform;
            const auto body_id = static_cast<BodyID>(comp.body_id);
            glm::vec3 p, s;
            glm::quat r;
            Util::decompose_mat4(tr, p, r, s);
            const auto pos = glm_vec3_to_jph_vec3(p);
            auto rot = glm_quat_to_jph_quat(r);
            auto scale = glm_vec3_to_jph_vec3(s);
            if (scale.IsNearZero())
            {
                LOG_INFO("scale too smol");
                scale = JPH::Vec3::sOne();
            }
            auto &body_interface = physics_system.GetBodyInterface();
            const auto shape = body_interface.GetShape(body_id);
            const auto new_shape = shape->ScaleShape(scale).Get();
            body_interface.SetShape(body_id, new_shape, true, EActivation::DontActivate);
            if (!rot.IsNormalized())
            {
                LOG_INFO("Quat isn't normalized!");
                rot = JPH::Quat::sIdentity();
            }
            body_interface.SetPositionAndRotation(body_id, pos, rot, EActivation::DontActivate);
        }
    }

    bool raycast(glm::vec3 origin, glm::vec3 direction, float max_distance, const std::vector<uint32_t>& layers, RaycastHitInfo &info)
    {
        RRayCast cast(glm_vec3_to_jph_vec3(origin), glm_vec3_to_jph_vec3(direction * max_distance));
        RayCastResult result{};
        RayCastLayerFilter filter(layers);
        if (!physics_system.GetNarrowPhaseQuery().CastRay(cast, result, {}, filter))
        {
            return false;
        }

        //HitPoint = Start + mFraction * (End - Start)
        const Vec3 outPosition = cast.mOrigin + result.mFraction * cast.mDirection;
        float mag = (outPosition + cast.mOrigin).Length();
        info.direction = direction;
        info.hit_length = mag;
        info.hit_point = jph_vec3_to_glm_vec3(outPosition);
        BodyLockRead lock(physics_system.GetBodyLockInterfaceNoLock(), result.mBodyID);
        if (lock.Succeeded())
        {
            const Body &hit_body = lock.GetBody();
            const Vec3 normal = hit_body.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, outPosition);
            info.hit_normal = jph_vec3_to_glm_vec3(normal);
        }
        else
        {
            info.hit_normal = glm::vec3(0.0f, 1.0f, 0.0f);
            return false;
        }
        info.hit_entity = entity_to_collider_map[result.mBodyID];
        return true;
    }


    void cleanup()
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

    void destroy_body(uint32_t body_id)
    {
        disable_body(body_id);
        physics_system.GetBodyInterface().DestroyBody(static_cast<BodyID>(body_id));
    }

    void disable_body(uint32_t body_id)
    {
        physics_system.GetBodyInterface().RemoveBody(static_cast<BodyID>(body_id));
    }

    void enable_body(uint32_t body_id)
    {
        physics_system.GetBodyInterface().AddBody(static_cast<BodyID>(body_id), EActivation::DontActivate);
    }
}
