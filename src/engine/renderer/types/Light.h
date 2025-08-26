#pragma once
#include <engine/scene/Components.h>

namespace cologne
{
    enum LightType
    {
        Directional = 0,
        Point = 1,
        Spot = 2
    };
#pragma pack(push, 1)
    struct Light
    {
        Light(LightComponent light_component, TransformComponent transform)
        {
            position = glm::vec4(transform.position, 1.0f);
            direction = glm::vec4(transform.get_forward(), 1.0f);
            if (abs(direction.z) < 0.0001f)
            {
                direction.z = 5.0f;
            }
            color = glm::vec4(light_component.color, 1.0f);
            strength = light_component.strength;
            radius = light_component.radius;
            type = static_cast<LightType>(light_component.type);
            active = 1;
            outer_cutoff = light_component.outer_cutoff;
            inner_cutoff = light_component.inner_cutoff;
            light_space_matrix = glm::mat4(1.0f);
        }

        bool is_similar(const Light &other)
        {
            return position == other.position &&
                   direction == other.direction &&
                   color == other.color &&
                   radius == other.radius &&
                   strength == other.strength &&
                   type == other.type &&
                   active == other.active &&
                   outer_cutoff == other.outer_cutoff &&
                   inner_cutoff == other.inner_cutoff;
        }

        bool is_similar(const LightComponent& comp)
        {
            return
                   color == glm::vec4(comp.color, 1.0f) &&
                   radius == comp.radius &&
                   strength == comp.strength &&
                   type == static_cast<LightType>(comp.type) &&
                   outer_cutoff == comp.outer_cutoff &&
                   inner_cutoff == comp.inner_cutoff;
        }

        Light() = default;

        Light(glm::vec3 pos, glm::vec3 direction, glm::vec3 color, float radius, float strength, LightType type)
        {
            position = glm::vec4(pos, 1.0f);
            this->direction = glm::vec4(direction, 1.0f);
            this->color = glm::vec4(color, 1.0f);
            this->radius = radius;
            this->strength = strength;
            this->type = type;
            active = 1;
        }

        glm::vec4 direction = glm::vec4(0.0f);
        glm::vec4 position = glm::vec4(0.0f);
        glm::vec4 color = glm::vec4(1, 0.7799999713897705, 0.5289999842643738, 1.0f);
        float strength = 0.0f;
        float radius = 6.0f;
        LightType type = LightType::Directional;
        int32_t active = 0;
        uint64_t shadow_handle = 0;
        float outer_cutoff = 17.5f;
        float inner_cutoff = 12.5f;
        glm::mat4 light_space_matrix = glm::mat4(1.0f);
    };
#pragma pack(pop)
}
