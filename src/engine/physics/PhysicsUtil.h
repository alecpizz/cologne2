//
// Created by alecpizz on 9/14/25.
//

#pragma once


namespace JPH
{
    class Quat;
    class Vec3;
    class Float3;
    class Quat;
}

namespace cologne::PhysicsUtil
{
    glm::vec3 jph_vec3_to_glm_vec3(const JPH::Vec3 &v);
    glm::quat jph_quat_to_glm_quat(const JPH::Quat &q);
    JPH::Vec3 glm_vec3_to_vec3(const glm::vec3 &v);
    JPH::Quat glm_quat_to_jph_quat(const glm::quat &q);
    JPH::Float3 glm_vec3_to_float3(const glm::vec3 &v);
    JPH::Vec3 glm_vec3_to_jph_vec3(const glm::vec3 &v);
}
