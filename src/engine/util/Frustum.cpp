//
// Created by alecpizz on 6/8/25.
//
#include "Frustum.h"

namespace cologne
{
    void Frustum::update(const glm::mat4 pv)
    {
        _planes[0].normal.x = pv[0][3] + pv[0][0];
        _planes[0].normal.y = pv[1][3] + pv[1][0];
        _planes[0].normal.z = pv[2][3] + pv[2][0];
        _planes[0].offset = pv[3][3] + pv[3][0];

        // Right clipping plane
        _planes[1].normal.x = pv[0][3] - pv[0][0];
        _planes[1].normal.y = pv[1][3] - pv[1][0];
        _planes[1].normal.z = pv[2][3] - pv[2][0];
        _planes[1].offset = pv[3][3] - pv[3][0];

        // Top clipping plane
        _planes[2].normal.x = pv[0][3] - pv[0][1];
        _planes[2].normal.y = pv[1][3] - pv[1][1];
        _planes[2].normal.z = pv[2][3] - pv[2][1];
        _planes[2].offset = pv[3][3] - pv[3][1];

        // Bottom clipping plane
        _planes[3].normal.x = pv[0][3] + pv[0][1];
        _planes[3].normal.y = pv[1][3] + pv[1][1];
        _planes[3].normal.z = pv[2][3] + pv[2][1];
        _planes[3].offset = pv[3][3] + pv[3][1];

        // Near clipping plane
        _planes[4].normal.x = pv[0][3] + pv[0][2];
        _planes[4].normal.y = pv[1][3] + pv[1][2];
        _planes[4].normal.z = pv[2][3] + pv[2][2];
        _planes[4].offset = pv[3][3] + pv[3][2];

        // Far clipping plane
        _planes[5].normal.x = pv[0][3] - pv[0][2];
        _planes[5].normal.y = pv[1][3] - pv[1][2];
        _planes[5].normal.z = pv[2][3] - pv[2][2];
        _planes[5].offset = pv[3][3] - pv[3][2];

        // Normalize planes
        for (int i = 0; i < 6; i++)
        {
            float magnitude = glm::length(_planes[i].normal);
            _planes[i].normal /= magnitude;
            _planes[i].offset /= magnitude;
        }
    }

    bool Frustum::intersect_point(const glm::vec3 point)
    {
        for (int i = 0; i < 6; i++)
        {
            float distance = glm::dot(_planes[i].normal, point) + _planes[i].offset;
            if (distance < 0)
            {
                return false;
            }
        }
        return true;
    }
}
