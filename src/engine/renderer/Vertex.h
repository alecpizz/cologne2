#pragma once
namespace goon
{
    struct Vertex
    {
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec2 uv = glm::vec2(0.0f, 0.0f);
        glm::vec3 tangent = glm::vec3(0.0f, 0.0f, 0.0f);

        Vertex() = default;
        Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv, glm::vec3 tangent)
        {
          this->position = position;
          this->normal = normal;
          this->uv = uv;
          this->tangent = tangent;
        }

        Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv)
        {
          this->position = position;
          this->normal = normal;
          this->uv = uv;
        }
        bool operator==(const Vertex& other) const
        {
              return position == other.position && normal == other.normal && uv == other.uv;
        }
    };
}
