//
// Created by alecp on 3/24/2025.
//

#include "Physics.h"

#include <engine/animation/SkeletonPose.h>
#include <engine/util/DebugScope.h>
#include "RaycastHitInfo.h"
#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Mesh.h>
#include <engine/scene/Components.h>
#include <engine/util/FileUtil.h>
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
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Jolt/Renderer/DebugRendererSimple.h>

#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <fstream>
#include "Jolt/Physics/Constraints/SwingTwistConstraint.h"
#include "Jolt/Physics/Ragdoll/Ragdoll.h"

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
        uint32_t _flags;

    public:
        explicit RayCastLayerFilter(uint32_t flags)
        {
            _flags = flags;
        }

        ~RayCastLayerFilter() override
        {
        }

        bool ShouldCollide(ObjectLayer inLayer) const override
        {
            if (_flags == 0)
            {
                return true;
            }
            uint8_t layer_byte = static_cast<uint8_t>(inLayer);
            for (int i = 0; i < 4; i++)
            {
                if (static_cast<uint8_t>(_flags >> (i * 8)) == layer_byte)
                {
                    return true;
                }
            }
            return false;
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


    JPH::Vec3 glm_vec3_to_vec3(const glm::vec3 &v)
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }

    JPH::Quat glm_quat_to_jph_quat(const glm::quat &q)
    {
        return JPH::Quat(q.x, q.y, q.z, q.w).Normalized();
    }

    RagdollSettings *create_ragdoll_settings(const Skeleton &skeleton,
                                             const std::vector<glm::mat4> &global_bind_transforms)
    {
        int hips_idx = skeleton.try_find_bone_index("Hips");
        int lower_spine_idx = skeleton.try_find_bone_index("Spine");
        int middle_spine_idx = skeleton.try_find_bone_index("Spine1");
        int upper_spine_idx = skeleton.try_find_bone_index("Spine2");
        int head_idx = skeleton.try_find_bone_index("Head");
        int left_arm_idx = skeleton.try_find_bone_index("LeftArm");
        int left_fore_arm_idx = skeleton.try_find_bone_index("LeftForeArm");
        int left_hand_idx = skeleton.try_find_bone_index("LeftHand");
        int right_arm_idx = skeleton.try_find_bone_index("RightArm");
        int right_fore_arm_idx = skeleton.try_find_bone_index("RightForeArm");
        int right_hand_idx = skeleton.try_find_bone_index("RightHand");
        int left_up_leg_idx = skeleton.try_find_bone_index("LeftUpLeg");
        int left_leg_idx = skeleton.try_find_bone_index("LeftLeg");
        int left_foot_idx = skeleton.try_find_bone_index("LeftFoot");
        int right_up_leg_idx = skeleton.try_find_bone_index("RightUpLeg");
        int right_leg_idx = skeleton.try_find_bone_index("RightLeg");
        int right_foot_idx = skeleton.try_find_bone_index("RightFoot");

        const Bone &hips_bone = skeleton.get_bones()[hips_idx];
        const Bone &lower_spine_bone = skeleton.get_bones()[lower_spine_idx];
        const Bone &middle_spine_bone = skeleton.get_bones()[middle_spine_idx];
        const Bone &upper_spine_bone = skeleton.get_bones()[upper_spine_idx];
        const Bone &head_bone = skeleton.get_bones()[head_idx];
        const Bone &left_arm_bone = skeleton.get_bones()[left_arm_idx];
        const Bone &left_fore_arm_bone = skeleton.get_bones()[left_fore_arm_idx];
        const Bone &left_hand_bone = skeleton.get_bones()[left_hand_idx];
        const Bone &right_arm_bone = skeleton.get_bones()[right_arm_idx];
        const Bone &right_fore_arm_bone = skeleton.get_bones()[right_fore_arm_idx];
        const Bone &right_hand_bone = skeleton.get_bones()[right_hand_idx];
        const Bone &left_up_leg_bone = skeleton.get_bones()[left_up_leg_idx];
        const Bone &left_leg_bone = skeleton.get_bones()[left_leg_idx];
        const Bone &left_foot_bone = skeleton.get_bones()[left_foot_idx];
        const Bone &right_up_leg_bone = skeleton.get_bones()[right_up_leg_idx];
        const Bone &right_leg_bone = skeleton.get_bones()[right_leg_idx];
        const Bone &right_foot_bone = skeleton.get_bones()[right_foot_idx];


        //build jolt skeleton
        Ref<JPH::Skeleton> j_skeleton = new JPH::Skeleton;
        uint hips = j_skeleton->AddJoint(hips_bone.name);
        uint lower_body = j_skeleton->AddJoint(lower_spine_bone.name, hips);
        uint mid_body = j_skeleton->AddJoint(middle_spine_bone.name, lower_body);
        uint upper_body = j_skeleton->AddJoint(upper_spine_bone.name, mid_body);
        j_skeleton->AddJoint(head_bone.name, upper_body);
        uint upper_arm_l = j_skeleton->AddJoint(left_arm_bone.name, upper_body);
        uint upper_arm_r = j_skeleton->AddJoint(right_arm_bone.name, upper_body);
        uint lower_arm_l = j_skeleton->AddJoint(left_fore_arm_bone.name, upper_arm_l);
        uint lower_arm_r = j_skeleton->AddJoint(right_fore_arm_bone.name, upper_arm_r);
        j_skeleton->AddJoint(left_hand_bone.name, lower_arm_l);
        j_skeleton->AddJoint(right_hand_bone.name, lower_arm_r);
        uint upper_leg_l = j_skeleton->AddJoint(left_up_leg_bone.name, hips);
        uint upper_leg_r = j_skeleton->AddJoint(right_up_leg_bone.name, hips);
        uint lower_leg_l = j_skeleton->AddJoint(left_leg_bone.name, upper_leg_l);
        uint lower_leg_r = j_skeleton->AddJoint(right_leg_bone.name, upper_leg_r);
        j_skeleton->AddJoint(left_foot_bone.name, lower_leg_l);
        j_skeleton->AddJoint(right_foot_bone.name, lower_leg_r);


        //positions are global_bind_transforms[idx][3]
        RVec3 positions[] = {
            glm_vec3_to_vec3(global_bind_transforms[hips_idx][3]), //Hips
            glm_vec3_to_vec3(global_bind_transforms[lower_spine_idx][3]), // Lower Body
            glm_vec3_to_vec3(global_bind_transforms[middle_spine_idx][3]), // Mid Body
            glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Upper Body
            glm_vec3_to_vec3(global_bind_transforms[head_idx][3]), // Head
            glm_vec3_to_vec3(global_bind_transforms[left_arm_idx][3]), // Upper Arm L
            glm_vec3_to_vec3(global_bind_transforms[right_arm_idx][3]), // Upper Arm R
            glm_vec3_to_vec3(global_bind_transforms[left_fore_arm_idx][3]), // Lower Arm L
            glm_vec3_to_vec3(global_bind_transforms[right_fore_arm_idx][3]), // Lower Arm R
            glm_vec3_to_vec3(global_bind_transforms[left_hand_idx][3]), //Hand L
            glm_vec3_to_vec3(global_bind_transforms[right_hand_idx][3]), //Hand R
            glm_vec3_to_vec3(global_bind_transforms[left_up_leg_idx][3]), // Upper Leg L
            glm_vec3_to_vec3(global_bind_transforms[right_up_leg_idx][3]), // Upper Leg R
            glm_vec3_to_vec3(global_bind_transforms[left_leg_idx][3]), // Lower Leg L
            glm_vec3_to_vec3(global_bind_transforms[right_leg_idx][3]), // Lower Leg R
            glm_vec3_to_vec3(global_bind_transforms[left_foot_idx][3]), //Foot L
            glm_vec3_to_vec3(global_bind_transforms[right_foot_idx][3]) //Foot R
        };


        //rotations are quat_cast(global_bind_transforms[idx][3]
        Quat rotations[] = {
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[hips_idx])), //Hips
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[lower_spine_idx])), // Lower Body
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[middle_spine_idx])), // Mid Body
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[upper_spine_idx])), // Upper Body
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[head_idx])), // Head
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_arm_idx])), // Upper Arm L
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_arm_idx])), // Upper Arm R
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_fore_arm_idx])), // Lower Arm L
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_fore_arm_idx])), // Lower Arm R
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_hand_idx])), //Hand L
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_hand_idx])), //Hand R
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_up_leg_idx])), // Upper Leg L
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_up_leg_idx])), // Upper Leg R
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_leg_idx])), // Lower Leg L
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_leg_idx])), // Lower Leg R
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_foot_idx])), //Foot L
            glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_foot_idx])) //Foot R
        };

        //length of a bone: upper_arm_l position - lower_arm_l position length
        Ref<Shape> shapes[] = {
            new CapsuleShape(
                glm::length(
                    global_bind_transforms[hips_idx][3] - global_bind_transforms[lower_spine_idx][3]) * 0.5,
                0.10f), //Hips
            new CapsuleShape(glm::length(
                                 global_bind_transforms[lower_spine_idx][3] - global_bind_transforms[middle_spine_idx][
                                     3]) * 0.5f,
                             0.10f), // Lower Body
            new CapsuleShape(glm::length(
                                 global_bind_transforms[middle_spine_idx][3] - global_bind_transforms[upper_spine_idx][
                                     3]) * 0.5f,
                             0.10f), // Mid Body
            new CapsuleShape(glm::length(
                                 global_bind_transforms[upper_spine_idx][3] - global_bind_transforms[head_idx][3]) *
                             0.5f,
                             0.10f), // Upper Body
            new CapsuleShape(0.075f, 0.10f), // Head
            new CapsuleShape(glm::length(
                                 global_bind_transforms[left_arm_idx][3] - global_bind_transforms[left_fore_arm_idx][3])
                             * 0.5f,
                             0.06f), // Upper Arm L
            new CapsuleShape(glm::length(
                                 global_bind_transforms[right_arm_idx][3] - global_bind_transforms[right_fore_arm_idx][
                                     3]) * 0.5f,
                             0.06f), // Upper Arm R
            new CapsuleShape(glm::length(
                                 global_bind_transforms[left_fore_arm_idx][3] - global_bind_transforms[left_hand_idx][
                                     3]) * 0.5f,
                             0.05f), // Lower Arm L
            new CapsuleShape(glm::length(
                                 global_bind_transforms[right_fore_arm_idx][3] - global_bind_transforms[right_hand_idx][
                                     3]) * 0.5f,
                             0.05f), // Lower Arm R
            new SphereShape(0.1f), //Hand L
            new SphereShape(0.1f), //Hand R
            new CapsuleShape(glm::length(
                                 global_bind_transforms[left_up_leg_idx][3] - global_bind_transforms[left_leg_idx][3]) *
                             0.5f,
                             0.075f), // Upper Leg L
            new CapsuleShape(glm::length(
                                 global_bind_transforms[right_up_leg_idx][3] - global_bind_transforms[right_leg_idx][3])
                             * 0.5f,
                             0.075f), // Upper Leg R
            new CapsuleShape(glm::length(
                                 global_bind_transforms[left_leg_idx][3] - global_bind_transforms[left_foot_idx][3]) *
                             0.5f,
                             0.06f), // Lower Leg L
            new CapsuleShape(glm::length(
                                 global_bind_transforms[right_leg_idx][3] - global_bind_transforms[right_foot_idx][3]) *
                             0.5f,
                             0.06f), // Lower Leg R
            new SphereShape(0.1f), //Foot L
            new SphereShape(0.1f) //Foot R
        };
        //constraint position1s are parent positions
        RVec3 constraint_position1s[] = {
            RVec3(0.0f, 0.0f, 0.0f), //hips (unused
            glm_vec3_to_vec3(global_bind_transforms[lower_spine_idx][3]), // Lower Body
            glm_vec3_to_vec3(global_bind_transforms[middle_spine_idx][3]), // Mid Body
            glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Upper Body
            glm_vec3_to_vec3(global_bind_transforms[head_idx][3]), // Head
            glm_vec3_to_vec3(global_bind_transforms[left_arm_idx][3]), // Upper Arm L
            glm_vec3_to_vec3(global_bind_transforms[right_arm_idx][3]), // Upper Arm R
            glm_vec3_to_vec3(global_bind_transforms[left_fore_arm_idx][3]), // Lower Arm L
            glm_vec3_to_vec3(global_bind_transforms[right_fore_arm_idx][3]), // Lower Arm R
            glm_vec3_to_vec3(global_bind_transforms[left_hand_idx][3]), //Hand L
            glm_vec3_to_vec3(global_bind_transforms[right_hand_idx][3]), //Hand R
            glm_vec3_to_vec3(global_bind_transforms[left_up_leg_idx][3]), // Upper Leg L
            glm_vec3_to_vec3(global_bind_transforms[right_up_leg_idx][3]), // Upper Leg R
            glm_vec3_to_vec3(global_bind_transforms[left_leg_idx][3]), // Lower Leg L
            glm_vec3_to_vec3(global_bind_transforms[right_leg_idx][3]), // Lower Leg R
            glm_vec3_to_vec3(global_bind_transforms[left_foot_idx][3]), //Foot L
            glm_vec3_to_vec3(global_bind_transforms[right_foot_idx][3]) //Foot R
        };

        //constraint position2s are child positions
        RVec3 constraint_position2s[] = {
            RVec3(0.0f, 0.0f, 0.0f), //hips (unused
            glm_vec3_to_vec3(global_bind_transforms[hips_idx][3]), // Lower Body
            glm_vec3_to_vec3(global_bind_transforms[lower_spine_idx][3]), // Mid Body
            glm_vec3_to_vec3(global_bind_transforms[middle_spine_idx][3]), // Upper Body
            glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Head
            glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Upper Arm L
            glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Upper Arm R
            glm_vec3_to_vec3(global_bind_transforms[left_arm_idx][3]), // Lower Arm L
            glm_vec3_to_vec3(global_bind_transforms[right_arm_idx][3]), // Lower Arm R
            glm_vec3_to_vec3(global_bind_transforms[left_fore_arm_idx][3]), //Hand L
            glm_vec3_to_vec3(global_bind_transforms[right_fore_arm_idx][3]), //Hand R
            glm_vec3_to_vec3(global_bind_transforms[hips_idx][3]), // Upper Leg L
            glm_vec3_to_vec3(global_bind_transforms[hips_idx][3]), // Upper Leg R
            glm_vec3_to_vec3(global_bind_transforms[left_up_leg_idx][3]), // Lower Leg L
            glm_vec3_to_vec3(global_bind_transforms[right_up_leg_idx][3]), // Lower Leg R
            glm_vec3_to_vec3(global_bind_transforms[left_leg_idx][3]), //Foot L
            glm_vec3_to_vec3(global_bind_transforms[right_leg_idx][3]) //Foot R
        };

        //twist axis is child - parent dir
        Vec3 twist_axis[] = {
            Vec3(0.0f, 0.0f, 0.0f), //hips (unused
            glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[middle_spine_idx][3] + global_bind_transforms[lower_spine_idx][3])),
            // Lower Body
            glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[upper_spine_idx][3] + global_bind_transforms[middle_spine_idx][3])),
            // Mid Body
            glm_vec3_to_vec3(
                glm::normalize(-global_bind_transforms[head_idx][3] + global_bind_transforms[upper_spine_idx][3])),
            // Upper Body
            glm_vec3_to_vec3(glm::vec3(0.0f, 1.0f, 0.0f)), // Head
            glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[left_fore_arm_idx][3] + global_bind_transforms[left_arm_idx][3])),
            // Upper Arm L
            glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[right_fore_arm_idx][3] + global_bind_transforms[right_arm_idx][3])),
            // Upper Arm R
            glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[left_hand_idx][3] + global_bind_transforms[left_fore_arm_idx][3])),
            // Lower Arm L
            glm_vec3_to_vec3(glm::normalize(
                -global_bind_transforms[right_hand_idx][3] + global_bind_transforms[right_fore_arm_idx][3])),
            // Lower Arm R
            -Vec3::sAxisX(), //Hand L
            Vec3::sAxisX(), //Hand R
            glm_vec3_to_vec3(
                glm::normalize(-global_bind_transforms[left_leg_idx][3] + global_bind_transforms[left_up_leg_idx][3])),
            // Upper Leg L
            glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[right_leg_idx][3] + global_bind_transforms[right_up_leg_idx][3])),
            // Upper Leg R
            glm_vec3_to_vec3(
                glm::normalize(-global_bind_transforms[left_foot_idx][3] + global_bind_transforms[left_leg_idx][3])),
            // Lower Leg L
            glm_vec3_to_vec3(
                glm::normalize(-global_bind_transforms[right_foot_idx][3] + global_bind_transforms[right_leg_idx][3])),
            // Lower Leg R
            -Vec3::sAxisY(), //Foot L
            -Vec3::sAxisY() //Foot R
        };


        //plane axis:
        //     get grand child pos (e.g. for an arm it'd be upper arm -> lower arm -> hand
        //then the swing vec would be grand_child - child
        // and then the plane axis would be cross(twist, swing)
        // and finally the normal axis would be cross(twist, plane)
        //the limits are gonna have to be vibe made.

        // Constraint limits
        float twist_angle[] = {
            0.0f, // Hips (unused, there's no parent)
            5.0f, // Lower Body
            5.0f, // Mid Body
            5.0f, // Upper Body
            90.0f, // Head
            45.0f, // Upper Arm L
            45.0f, // Upper Arm R
            45.0f, // Lower Arm L
            45.0f, // Lower Arm R
            45.0f, // Hand L
            45.0f, // Hand R
            45.0f, // Upper Leg L
            45.0f, // Upper Leg R
            45.0f, // Lower Leg L
            45.0f, // Lower Leg R
            45.0f, // Foot L
            45.0f, // Foot R
        };

        //done
        float normal_angle[] = {
            0.0f, // Hips (unused, there's no parent)
            10.0f, // Lower Body
            10.0f, // Mid Body
            10.0f, // Upper Body
            45.0f, // Head
            90.0f, // Upper Arm L
            90.0f, // Upper Arm R
            0.0f, // Lower Arm L
            0.0f, // Lower Arm R
            0.0f, // Hand L
            0.0f, // Hand R
            45.0f, // Upper Leg L
            45.0f, // Upper Leg R
            0.0f, // Lower Leg L
            0.0f, // Lower Leg R
            0.0f, // Foot L
            0.0f, // Foot R
        };

        //NOT DONE
        float plane_angle[] = {
            0.0f, // hips (unused, there's no parent)
            10.0f, // Lower Body
            10.0f, // Mid Body
            10.0f, // Upper Body
            45.0f, // Head
            45.0f, // Upper Arm L
            45.0f, // Upper Arm R
            90.0f, // Lower Arm L
            90.0f, // Lower Arm R
            15.0f, // Hand L
            15.0f, // Hand R
            45.0f, // Upper Leg L
            45.0f, // Upper Leg R
            60.0f, // Lower Leg L (cheating here, a knee is not symmetric, we should have rotated the twist axis)
            60.0f, // Lower Leg R
            15.0f, // Foot L
            15.0f, // Foot R
        };

        // Create ragdoll settings
        auto settings = new RagdollSettings;
        settings->mSkeleton = j_skeleton;
        settings->mParts.resize(j_skeleton->GetJointCount());
        for (int p = 0; p < j_skeleton->GetJointCount(); ++p)
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
                constraint->mPosition1 = constraint_position1s[p];
                constraint->mPosition2 = constraint_position1s[p];
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
    BodyDrawFilterImpl body_draw_filter;
    PhysDebugRenderer *debug_renderer = nullptr;
    std::vector<JPH::BodyID> colliders_static;
    std::unordered_map<JPH::BodyID, Entity> entity_to_collider_map;
    std::unordered_map<uint32_t, PhysicsPlayer> physics_players;
    std::vector<JPH::Ragdoll *> ragdolls;
    bool drawing = false;


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

    float accumulation_time = 0.0f;
    float fixed_delta_time = 1.0 / 60.0f;

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
            const int collisionSteps = 4;
            accumulation_time += dt;
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


    glm::vec3 jph_vec3_to_glm_vec3(const JPH::Vec3 &v)
    {
        return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
    }

    glm::quat jph_quat_to_glm_quat(const JPH::Quat &q)
    {
        return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
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
                                         const Mesh &mesh)
    {
        JPH::Ref<Shape> mesh_shape;
        const std::string path = RESOURCES_PATH "cache/colliders/" + mesh.get_name() + ".ccol";
        if (!FileUtil::file_exists(path))
        {
            FileUtil::create_directory_recursive(path);
            LOG_INFO("No cache collider for %s mesh! Generating one.", mesh.get_name().c_str());
            JPH::TriangleList triangle_list;
            for (int i = 0; i * 3 < mesh.get_indices_count(); i++)
            {
                Triangle triangle =
                {
                    glm_vec3_to_float3(mesh.get_vertices()[mesh.get_indices()[3 * i]].position),
                    glm_vec3_to_float3(mesh.get_vertices()[mesh.get_indices()[3 * i + 1]].position),
                    glm_vec3_to_float3(mesh.get_vertices()[mesh.get_indices()[3 * i + 2]].position)
                };
                triangle_list.emplace_back(triangle);
            }
            JPH::MeshShapeSettings mesh_settings(triangle_list);
            mesh_settings.mMaxTrianglesPerLeaf = 4;
            mesh_settings.mBuildQuality = MeshShapeSettings::EBuildQuality::FavorRuntimePerformance;
            mesh_settings.SetEmbedded();
            auto result = mesh_settings.Create();
            mesh_shape = result.Get();
            //export now
            std::ofstream file(path, std::ios::binary);
            if (!file.is_open())
            {
                LOG_ERROR("Couldn't open file to export collision shape!");
                return -1;
            }
            JPH::StreamOutWrapper stream_out(file);
            JPH::Shape::ShapeToIDMap shape_to_id_map;
            JPH::Shape::MaterialToIDMap material_to_id_map;
            mesh_shape->SaveWithChildren(stream_out, shape_to_id_map, material_to_id_map);
            file.close();
        }
        else
        {
            // settings = something_else;
            std::ifstream file(path, std::ios::binary);
            if (!file.is_open())
            {
                LOG_ERROR("Couldn't open file to import collision shape!");
                return -1;
            }
            JPH::StreamInWrapper stream_in(file);
            JPH::Shape::IDToShapeMap id_to_shape_map;
            JPH::Shape::IDToMaterialMap id_to_material_map;
            JPH::Shape::ShapeResult result = JPH::Shape::sRestoreWithChildren(
                stream_in, id_to_shape_map, id_to_material_map);
            file.close();
            if (result.IsValid())
            {
                mesh_shape = result.Get();
            }
            else
            {
                LOG_ERROR("Couldn't open shape from file!");
                return -1;
            }
        }
        auto quat = glm_quat_to_jph_quat(transform.rotation);
        if (!quat.IsNormalized())
        {
            LOG_INFO("Quat isn't normalized!");
            quat = quat.sIdentity();
        }
        auto settings = BodyCreationSettings(new ScaledShapeSettings(mesh_shape,
                                                                     glm_vec3_to_jph_vec3(transform.scale)),
                                             glm_vec3_to_jph_vec3(transform.position), quat, JPH::EMotionType::Static,
                                             cologne::Physics::NON_MOVING);

        auto &body_interface = physics_system.GetBodyInterface();
        auto id = body_interface.CreateAndAddBody(
            settings, JPH::EActivation::DontActivate);
        LOG_INFO("Created collider with id %d", id);
        colliders_static.push_back(id);
        physics_system.OptimizeBroadPhase();
        entity_to_collider_map[id] = entity;
        return id.GetIndexAndSequenceNumber();
    }

    uint32_t create_infinite_ground_plane(glm::vec3 plane_normal, float constant)
    {
        auto id = physics_system.GetBodyInterface().CreateAndAddBody(
            BodyCreationSettings(
                new PlaneShape(Plane(JPH::Vec3(plane_normal.x, plane_normal.y, plane_normal.z).Normalized(), constant),
                               nullptr, 100), RVec3(0, 0, 0), Quat::sIdentity(), EMotionType::Static,
                Layers::NON_MOVING), EActivation::DontActivate);
        colliders_static.emplace_back(id);
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
            BodyLockWrite lock(physics_system.GetBodyLockInterface(), body_id);
            if (lock.Succeeded())
            {
                Body &body = lock.GetBody();
                const Shape *non_scaled_shape;
                if (body.GetShape()->GetSubType() != EShapeSubType::Scaled)
                {
                    non_scaled_shape = body.GetShape();
                }
                else
                {
                    const auto *scaled_shape = reinterpret_cast<const ScaledShape *>(body.GetShape());
                    non_scaled_shape = scaled_shape->GetInnerShape();
                    if (!non_scaled_shape)
                    {
                        LOG_ERROR("no internal shape!");
                        return;
                    }
                }
                Shape::ShapeResult new_shape = non_scaled_shape->ScaleShape(scale);
                physics_system.GetBodyInterfaceNoLock().SetShape(body.GetID(), new_shape.Get(), false,
                                                                 EActivation::DontActivate);
                physics_system.GetBodyInterfaceNoLock().SetPositionAndRotation(
                    body_id, pos, rot, EActivation::DontActivate);
            }
        }
    }

    bool raycast(glm::vec3 origin, glm::vec3 direction, float max_distance, uint32_t layers, RaycastHitInfo &info)
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

    void delete_all_bodies()
    {
        physics_system.GetBodyInterface().RemoveBodies(colliders_static.data(), colliders_static.size());
        physics_system.GetBodyInterface().DestroyBodies(colliders_static.data(), colliders_static.size());
        physics_players.clear();
        colliders_static.clear();
    }


    void cleanup()
    {
        delete_all_bodies();
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
        std::erase(colliders_static, static_cast<BodyID>(body_id));
    }

    void add_impulse_force_at_position(uint32_t body_id, glm::vec3 position, glm::vec3 force)
    {
        BodyID id(body_id);
        if (id.IsInvalid())
        {
            return;
        }

        physics_system.GetBodyInterface().AddImpulse(id, glm_vec3_to_jph_vec3(force), glm_vec3_to_jph_vec3(position));
    }

    void disable_body(uint32_t body_id)
    {
        physics_system.GetBodyInterface().RemoveBody(static_cast<BodyID>(body_id));
    }

    void enable_body(uint32_t body_id)
    {
        physics_system.GetBodyInterface().AddBody(static_cast<BodyID>(body_id), EActivation::DontActivate);
    }

    uint32_t create_ragdoll(Entity entity, std::unordered_map<std::string, uint32_t> &out_map, const Skeleton &skeleton,
                            const std::vector<glm::mat4> &global_bind_transforms)
    {
        Ref<RagdollSettings> settings = create_ragdoll_settings(skeleton, global_bind_transforms);
        auto ragdoll = settings->CreateRagdoll(0, 0, &physics_system);
        ragdoll->AddToPhysicsSystem(EActivation::Activate); //?
        uint32_t size = ragdolls.size();
        ragdolls.emplace_back(ragdoll);
        for (size_t i = 0; i < settings->GetSkeleton()->GetJointCount(); i++)
        {
            out_map[settings->GetSkeleton()->GetJoint(i).mName.c_str()] = ragdoll->GetBodyID(i).
                    GetIndexAndSequenceNumber();
            entity_to_collider_map[ragdoll->GetBodyID(i)] = entity;
        }
        return size;
    }

    void make_ragdoll_kinematic(uint32_t ragdoll_id)
    {
        auto *ragdoll = ragdolls[ragdoll_id];
        if (!ragdoll)
        {
            LOG_ERROR("No ragdoll found at idx %d", ragdoll_id);
            return;
        }
        for (size_t i = 0; i < ragdoll->GetBodyCount(); i++)
        {
            auto bodyId = ragdoll->GetBodyID(i);
            if (bodyId.IsInvalid())
            {
                continue;
            }
            physics_system.GetBodyInterface().SetMotionType(bodyId, EMotionType::Kinematic, EActivation::DontActivate);
        }
    }

    void make_ragdoll_active(uint32_t ragdoll_id)
    {
        auto *ragdoll = ragdolls[ragdoll_id];
        if (!ragdoll)
        {
            LOG_ERROR("No ragdoll found at idx %d", ragdoll_id);
            return;
        }
        for (size_t i = 0; i < ragdoll->GetBodyCount(); i++)
        {
            auto bodyId = ragdoll->GetBodyID(i);
            if (bodyId.IsInvalid())
            {
                continue;
            }
            physics_system.GetBodyInterface().SetMotionType(bodyId, EMotionType::Dynamic, EActivation::Activate);
        }
    }

    void sync_ragdoll(uint32_t ragdoll_id, const std::unordered_map<std::string, glm::mat4> &ragdoll_transforms)
    {
        auto *ragdoll = ragdolls[ragdoll_id];
        if (!ragdoll)
        {
            LOG_ERROR("No ragdoll found at idx %d", ragdoll_id);
            return;
        }
        auto skel = ragdoll->GetRagdollSettings()->GetSkeleton();
        for (size_t i = 0; i < ragdoll->GetBodyCount(); i++)
        {
            auto jt = skel->GetJoint(i);
            auto tr = ragdoll_transforms.at(jt.mName.c_str());
            glm::vec3 position;
            glm::quat rotation;
            glm::vec3 scale;
            Util::decompose_mat4(tr, position, rotation, scale);
            auto pos = glm_vec3_to_vec3(position);
            auto rot = glm_quat_to_jph_quat(rotation);
            physics_system.GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyID(i), pos, rot,
                                                                     EActivation::DontActivate);
        }
    }

    glm::mat4 get_rigidbody_transform(uint32_t body_id)
    {
        BodyID id = BodyID(body_id);
        if (id.IsInvalid())
        {
            LOG_ERROR("Unknown body!");
            return glm::mat4(1.0f);
        }

        auto jolt_mat = physics_system.GetBodyInterface().GetWorldTransform(BodyID(body_id));
        auto quat = jolt_mat.GetRotation().GetQuaternion();
        auto pos = jolt_mat.GetTranslation();

        glm::mat4 to = glm::mat4(1.0f);
        to = glm::translate(to, jph_vec3_to_glm_vec3(pos));
        to *= glm::toMat4((jph_quat_to_glm_quat(quat)));
        to = glm::scale(to, glm::vec3(1.0f));
        // to[0][0] = jolt_mat(0, 0);
        // to[1][0] = jolt_mat(1, 0);
        // to[2][0] = jolt_mat(2, 0);
        // to[3][0] = jolt_mat(3, 0);
        // to[0][1] = jolt_mat(0, 1);
        // to[1][1] = jolt_mat(1, 1);
        // to[2][1] = jolt_mat(2, 1);
        // to[3][1] = jolt_mat(3, 1);
        // to[0][2] = jolt_mat(0, 2);
        // to[1][2] = jolt_mat(1, 2);
        // to[2][2] = jolt_mat(2, 2);
        // to[3][2] = jolt_mat(3, 2);
        // to[0][3] = jolt_mat(0, 3);
        // to[1][3] = jolt_mat(1, 3);
        // to[2][3] = jolt_mat(2, 3);
        // to[3][3] = jolt_mat(3, 3);
        return to;
    }
}
