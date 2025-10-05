#include "Physics.h"
#include "PhysicsUtil.h"
#include "RaycastHitInfo.h"
#include <engine/scene/components/StaticColliderComponent.h>
#include <engine/scene/components/WorldTransformComponent.h>
#include <engine/util/Util.h>

#include <Jolt/Jolt.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Renderer/DebugRendererSimple.h>

#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include "Jolt/Physics/Ragdoll/Ragdoll.h"

//
// Created by alecpizz on 9/14/25.
//
namespace cologne
{
    using namespace JPH;

    void Physics::sync_transform(Entity entity)
    {
        if (entity.has_component<StaticColliderComponent>())
        {
            auto &comp = entity.get_component<StaticColliderComponent>();
            auto tr = entity.get_component<WorldTransformComponent>().transform;
            const auto body_id = static_cast<BodyID>(comp.body_id);
            glm::vec3 p, s;
            glm::quat r;
            Util::decompose_mat4(tr, p, r, s);
            const auto pos = PhysicsUtil::glm_vec3_to_jph_vec3(p);
            auto rot = PhysicsUtil::glm_quat_to_jph_quat(r);
            auto scale = PhysicsUtil::glm_vec3_to_jph_vec3(s);
            if (scale.IsNearZero())
            {
                LOG_INFO("scale too smol");
                scale = JPH::RVec3::sOne();
            }
            BodyLockWrite lock(_physics_system.GetBodyLockInterface(), body_id);
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
                _physics_system.GetBodyInterfaceNoLock().SetShape(body.GetID(), new_shape.Get(), false,
                                                                 EActivation::DontActivate);
                _physics_system.GetBodyInterfaceNoLock().SetPositionAndRotation(
                    body_id, pos, rot, EActivation::DontActivate);
            }
        }
    }

    glm::mat4 Physics::get_rigidbody_transform(uint32_t body_id)
    {
        BodyID id = BodyID(body_id);
        if (id.IsInvalid())
        {
            LOG_ERROR("Unknown body!");
            return glm::mat4(1.0f);
        }

        auto jolt_mat = _physics_system.GetBodyInterface().GetWorldTransform(BodyID(body_id));
        JPH::Quat quat = jolt_mat.GetRotation().GetQuaternion();
        auto pos = jolt_mat.GetTranslation();

        glm::mat4 to = glm::mat4(1.0f);
        to = glm::translate(to, PhysicsUtil::jph_vec3_to_glm_vec3(pos));
        to *= glm::toMat4((PhysicsUtil::jph_quat_to_glm_quat(quat)));
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