#pragma once

namespace cologne
{
    struct AABB
    {
        glm::vec4 min;
        glm::vec4 max;

        AABB() : min(glm::vec4(std::numeric_limits<float>::infinity())),
                           max(-std::numeric_limits<float>::infinity())
        {
        }

        AABB(const glm::vec4 &min, const glm::vec4 &max);

        bool valid() const;

        void expand(const glm::vec4 &pt);

        void expand(const glm::vec4 *pts, size_t count);

        void intersect(const AABB &o);

        void union_aabb(const AABB &o);

        void from_center_size(const glm::vec4 &center, const glm::vec4 &size)
        {
            glm::vec4 half = 0.5f * size;
            min = center - half;
            max = center + half;
        }

        AABB transform_by_mat4(glm::mat4 mat) const
        {
            const auto center = (max + min) * 0.5f;
            const auto extents = (max - min) * 0.5f;

            const auto world_center = glm::vec4(mat * center);
            const auto world_extents = abs(extents.x * mat[0]) + abs(extents.y * mat[1]) + abs(extents.z * mat[2]);

            const auto world_min = world_center - world_extents;
            const auto world_max = world_center + world_extents;
            return {world_min, world_max};
        }

        glm::vec4 center() const
        {
            return 0.5f * (min + max);
        }

        glm::vec4 size() const
        {
            return max - min;
        }
    };

    inline bool AABB::valid() const
    {
        bool result = (min.x < max.x);
        result = result && (min.y < max.y);
        result = result && (min.z < max.z);
        return result;
    }

    inline void AABB::expand(const glm::vec4 &pt)
    {
        min = glm::min(min, pt);
        max = glm::max(max, pt);
    }

    inline void AABB::expand(const glm::vec4 *pts, size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            expand(pts[i]);
        }
    }

    inline void AABB::intersect(const AABB &o)
    {
        min = glm::max(min, o.min);
        max = glm::min(max, o.max);
    }

    inline void AABB::union_aabb(const AABB &o)
    {
        min = glm::min(min, o.min);
        max = glm::max(max, o.max);
    }

    inline AABB::AABB(const glm::vec4 &min, const glm::vec4 &max)
    {
        this->min = min;
        this->max = max;
    }
}
