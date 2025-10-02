#include "Physics.h"
#include "PhysicsUtil.h"
#include "RaycastHitInfo.h"
#include <engine/renderer/Renderer.h>
#include <engine/scene/Components/Components.h>

#include <engine/util/FileUtil.h>
#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#include <fstream>
#include "Jolt/Physics/Constraints/SwingTwistConstraint.h"
#include "Jolt/Physics/Ragdoll/Ragdoll.h"

//
// Created by alecpizz on 9/14/25.
//
namespace cologne
{
    using namespace JPH;

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
        JPH::Ref<JPH::Skeleton> j_skeleton = new JPH::Skeleton;
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
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[hips_idx][3]), //Hips
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[lower_spine_idx][3]), // Lower Body
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[middle_spine_idx][3]), // Mid Body
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Upper Body
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[head_idx][3]), // Head
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_arm_idx][3]), // Upper Arm L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_arm_idx][3]), // Upper Arm R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_fore_arm_idx][3]), // Lower Arm L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_fore_arm_idx][3]), // Lower Arm R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_hand_idx][3]), //Hand L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_hand_idx][3]), //Hand R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_up_leg_idx][3]), // Upper Leg L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_up_leg_idx][3]), // Upper Leg R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_leg_idx][3]), // Lower Leg L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_leg_idx][3]), // Lower Leg R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_foot_idx][3]), //Foot L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_foot_idx][3]) //Foot R
        };


        //rotations are quat_cast(global_bind_transforms[idx][3]
        JPH::Quat rotations[] = {
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[hips_idx])), //Hips
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[lower_spine_idx])), // Lower Body
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[middle_spine_idx])), // Mid Body
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[upper_spine_idx])), // Upper Body
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[head_idx])), // Head
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_arm_idx])), // Upper Arm L
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_arm_idx])), // Upper Arm R
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_fore_arm_idx])), // Lower Arm L
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_fore_arm_idx])), // Lower Arm R
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_hand_idx])), //Hand L
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_hand_idx])), //Hand R
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_up_leg_idx])), // Upper Leg L
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_up_leg_idx])), // Upper Leg R
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_leg_idx])), // Lower Leg L
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_leg_idx])), // Lower Leg R
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[left_foot_idx])), //Foot L
            PhysicsUtil::glm_quat_to_jph_quat(glm::quat_cast(global_bind_transforms[right_foot_idx])) //Foot R
        };

        //length of a bone: upper_arm_l position - lower_arm_l position length
        JPH::Ref<Shape> shapes[] = {
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
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[lower_spine_idx][3]), // Lower Body
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[middle_spine_idx][3]), // Mid Body
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Upper Body
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[head_idx][3]), // Head
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_arm_idx][3]), // Upper Arm L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_arm_idx][3]), // Upper Arm R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_fore_arm_idx][3]), // Lower Arm L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_fore_arm_idx][3]), // Lower Arm R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_hand_idx][3]), //Hand L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_hand_idx][3]), //Hand R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_up_leg_idx][3]), // Upper Leg L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_up_leg_idx][3]), // Upper Leg R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_leg_idx][3]), // Lower Leg L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_leg_idx][3]), // Lower Leg R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_foot_idx][3]), //Foot L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_foot_idx][3]) //Foot R
        };

        //constraint position2s are child positions
        RVec3 constraint_position2s[] = {
            RVec3(0.0f, 0.0f, 0.0f), //hips (unused
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[hips_idx][3]), // Lower Body
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[lower_spine_idx][3]), // Mid Body
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[middle_spine_idx][3]), // Upper Body
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Head
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Upper Arm L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[upper_spine_idx][3]), // Upper Arm R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_arm_idx][3]), // Lower Arm L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_arm_idx][3]), // Lower Arm R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_fore_arm_idx][3]), //Hand L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_fore_arm_idx][3]), //Hand R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[hips_idx][3]), // Upper Leg L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[hips_idx][3]), // Upper Leg R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_up_leg_idx][3]), // Lower Leg L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_up_leg_idx][3]), // Lower Leg R
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[left_leg_idx][3]), //Foot L
            PhysicsUtil::glm_vec3_to_vec3(global_bind_transforms[right_leg_idx][3]) //Foot R
        };

        //twist axis is child - parent dir
        Vec3 twist_axis[] = {
            Vec3(0.0f, 0.0f, 0.0f), //hips (unused
            PhysicsUtil::glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[middle_spine_idx][3] + global_bind_transforms[lower_spine_idx][3])),
            // Lower Body
            PhysicsUtil::glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[upper_spine_idx][3] + global_bind_transforms[middle_spine_idx][3])),
            // Mid Body
            PhysicsUtil::glm_vec3_to_vec3(
                glm::normalize(-global_bind_transforms[head_idx][3] + global_bind_transforms[upper_spine_idx][3])),
            // Upper Body
            PhysicsUtil::glm_vec3_to_vec3(glm::vec3(0.0f, 1.0f, 0.0f)), // Head
            PhysicsUtil::glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[left_fore_arm_idx][3] + global_bind_transforms[left_arm_idx][3])),
            // Upper Arm L
            PhysicsUtil::glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[right_fore_arm_idx][3] + global_bind_transforms[right_arm_idx][3])),
            // Upper Arm R
            PhysicsUtil::glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[left_hand_idx][3] + global_bind_transforms[left_fore_arm_idx][3])),
            // Lower Arm L
            PhysicsUtil::glm_vec3_to_vec3(glm::normalize(
                -global_bind_transforms[right_hand_idx][3] + global_bind_transforms[right_fore_arm_idx][3])),
            // Lower Arm R
            -Vec3::sAxisX(), //Hand L
            Vec3::sAxisX(), //Hand R
            PhysicsUtil::glm_vec3_to_vec3(
                glm::normalize(-global_bind_transforms[left_leg_idx][3] + global_bind_transforms[left_up_leg_idx][3])),
            // Upper Leg L
            PhysicsUtil::glm_vec3_to_vec3(
                glm::normalize(
                    -global_bind_transforms[right_leg_idx][3] + global_bind_transforms[right_up_leg_idx][3])),
            // Upper Leg R
            PhysicsUtil::glm_vec3_to_vec3(
                glm::normalize(-global_bind_transforms[left_foot_idx][3] + global_bind_transforms[left_leg_idx][3])),
            // Lower Leg L
            PhysicsUtil::glm_vec3_to_vec3(
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

    uint32_t Physics::create_ragdoll(Entity entity, std::unordered_map<std::string, uint32_t> &out_map, const Skeleton &skeleton,
                            const std::vector<glm::mat4> &global_bind_transforms)
    {
        JPH::Ref<RagdollSettings> settings = create_ragdoll_settings(skeleton, global_bind_transforms);
        auto ragdoll = settings->CreateRagdoll(0, 0, &_physics_system);
        ragdoll->AddToPhysicsSystem(EActivation::Activate); //?
        uint32_t size = _ragdolls.size();
        _ragdolls.emplace_back(ragdoll);
        for (size_t i = 0; i < settings->GetSkeleton()->GetJointCount(); i++)
        {
            out_map[settings->GetSkeleton()->GetJoint(i).mName.c_str()] = ragdoll->GetBodyID(i).
                    GetIndexAndSequenceNumber();
            _entity_to_collider_map[ragdoll->GetBodyID(i)] = entity;
        }
        return size;
    }

    void Physics::make_ragdoll_kinematic(uint32_t ragdoll_id)
    {
        auto *ragdoll = _ragdolls[ragdoll_id];
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
            BodyLockWrite lock(_physics_system.GetBodyLockInterface(), bodyId);
            if (lock.Succeeded())
            {
                lock.GetBody().SetIsSensor(true);
                _physics_system.GetBodyInterfaceNoLock().SetMotionType(bodyId, EMotionType::Static, EActivation::DontActivate);
            }
        }
    }

    void Physics::make_ragdoll_active(uint32_t ragdoll_id)
    {
        auto *ragdoll = _ragdolls[ragdoll_id];
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
            BodyLockWrite lock(_physics_system.GetBodyLockInterface(), bodyId);
            if (lock.Succeeded())
            {
                lock.GetBody().SetIsSensor(false);
                _physics_system.GetBodyInterfaceNoLock().SetMotionType(bodyId, EMotionType::Dynamic, EActivation::Activate);
            }
        }
    }

    void Physics::sync_ragdoll(uint32_t ragdoll_id, const std::unordered_map<std::string, glm::mat4> &ragdoll_transforms)
    {
        auto *ragdoll = _ragdolls[ragdoll_id];
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
            auto pos = PhysicsUtil::glm_vec3_to_vec3(position);
            auto rot = PhysicsUtil::glm_quat_to_jph_quat(rotation);
            _physics_system.GetBodyInterface().SetPositionAndRotation(ragdoll->GetBodyID(i), pos, rot,
                                                                     EActivation::DontActivate);
        }
    }
}
