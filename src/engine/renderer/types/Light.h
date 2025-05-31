#pragma once
namespace cologne
{
    enum LightType
    {
         Directional = 0,
         Point
    };

    struct Light
    {
        Light() = default;
        Light(glm::vec3 pos, glm::vec3 direction, glm::vec3 color, float radius, float strength, LightType type)
        {
            position = pos;
            this->direction = direction;
            this->color = color;
            this->radius = radius;
            this->strength = strength;
            this->type = type;
        }
        glm::vec3 direction;
        glm::vec3 position;
        float strength = 1.0f;
        glm::vec3 color = glm::vec3(1, 0.7799999713897705, 0.5289999842643738);
        float radius = 6.0f;
        LightType type = LightType::Directional;

    };
}