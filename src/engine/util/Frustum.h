#pragma once

namespace cologne
{
    struct FrustumPlane
    {
        glm::vec3 normal;
        float offset;
    };
    struct Frustum
    {
        void update(const glm::mat4 pv);
        bool intersect_point(const glm::vec3 point);
    private:
        FrustumPlane _planes[6] = {};
    };
}
