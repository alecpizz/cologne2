#include "Physics.h"
#include "PhysicsUtil.h"
#include "RaycastHitInfo.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>

//
// Created by alecpizz on 9/14/25.
//
namespace cologne
{
    using namespace JPH;
    using namespace JPH::literals;
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


    bool Physics::raycast(glm::vec3 origin, glm::vec3 direction, float max_distance, uint32_t layers, RaycastHitInfo &info)
    {
        using namespace JPH;
        using namespace JPH::literals;
        RRayCast cast(PhysicsUtil::glm_vec3_to_jph_vec3(origin),PhysicsUtil:: glm_vec3_to_jph_vec3(direction * max_distance));
        RayCastResult result{};
        RayCastLayerFilter filter(layers);
        if (!_physics_system.GetNarrowPhaseQuery().CastRay(cast, result, {}, filter))
        {
            return false;
        }

        //HitPoint = Start + mFraction * (End - Start)
        const JPH::Vec3 outPosition = cast.mOrigin + result.mFraction * cast.mDirection;
        float mag = (outPosition + cast.mOrigin).Length();
        info.direction = direction;
        info.hit_length = mag;
        info.hit_point = PhysicsUtil::jph_vec3_to_glm_vec3(outPosition);
        BodyLockRead lock(_physics_system.GetBodyLockInterfaceNoLock(), result.mBodyID);
        if (lock.Succeeded())
        {
            const Body &hit_body = lock.GetBody();
            const JPH::Vec3 normal = hit_body.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, outPosition);
            info.hit_normal = PhysicsUtil::jph_vec3_to_glm_vec3(normal);
        }
        else
        {
            info.hit_normal = glm::vec3(0.0f, 1.0f, 0.0f);
            return false;
        }
        info.hit_entity = _entity_to_collider_map[result.mBodyID];
        return true;
    }
}