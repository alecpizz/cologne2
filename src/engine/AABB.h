#pragma once

namespace cologne
{
    struct AABB
    {
        glm::vec3 min{}, max{};

        AABB() : min(glm::vec3(std::numeric_limits<float>::infinity())),
                           max(-std::numeric_limits<float>::infinity())
        {
        }

        AABB(const glm::vec3 &min, const glm::vec3 &max);

        bool valid() const;

        void expand(const glm::vec3 &pt);

        void expand(const glm::vec3 *pts, size_t count);

        void intersect(const AABB &o);

        void union_aabb(const AABB &o);

        void from_center_size(const glm::vec3 &center, const glm::vec3 &size)
        {
            glm::vec3 half = 0.5f * size;
            min = center - half;
            max = center + half;
        }

        glm::vec3 center() const
        {
            return 0.5f * (min + max);
        }

        glm::vec3 size() const
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

    inline void AABB::expand(const glm::vec3 &pt)
    {
        min = glm::min(min, pt);
        max = glm::max(max, pt);
    }

    inline void AABB::expand(const glm::vec3 *pts, size_t count)
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

    inline AABB::AABB(const glm::vec3 &min, const glm::vec3 &max)
    {
        this->min = min;
        this->max = max;
    }
}
