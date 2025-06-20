#pragma once
#include <engine/AABB.h>

namespace cologne
{
    class Model
    {
    public:
        Model() = default;

        Model(const std::vector<int32_t>& meshes, const std::string& name, const glm::vec3& min, const glm::vec3& max);

        ~Model();

        AABB get_aabb() const;

        const char *get_name() const;

        std::vector<int32_t>& get_mesh_indices();

        void set_active(bool active);

        void set_aabb(AABB aabb);

        bool get_gi_only() const;

        bool get_cast_shadows() const;

        void set_cast_shadows(bool b);

        void set_gi_only(bool b);

    private:
        std::vector<int32_t> _mesh_indices = std::vector<int32_t>();
        bool _active = true;
        bool _cast_shadows = true;
        bool _gi_only = false;
        std::string _name;
        AABB _bounds;
    };
}
