//
// Created by alecpizz on 9/14/25.
//
#include "PhysicsUtil.h"
#include <Jolt/Jolt.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Math/Float3.h>
namespace cologne::PhysicsUtil
{
    glm::vec3 jph_vec3_to_glm_vec3(const JPH::Vec3 &v)
    {
        return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
    }

    glm::quat jph_quat_to_glm_quat(const JPH::Quat &q)
    {
        return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
    }

    JPH::Vec3 glm_vec3_to_vec3(const glm::vec3 &v)
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }

    JPH::Quat glm_quat_to_jph_quat(const glm::quat &q)
    {
        return JPH::Quat(q.x, q.y, q.z, q.w).Normalized();
    }

    JPH::Float3 glm_vec3_to_float3(const glm::vec3 &v)
    {
        return JPH::Float3(v.x, v.y, v.z);
    }

    JPH::Vec3 glm_vec3_to_jph_vec3(const glm::vec3 &v)
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }
}
