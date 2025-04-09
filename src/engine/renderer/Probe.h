#pragma once
namespace goon
{
    class Shader;
}

namespace goon
{
    class Probe
    {
    public:
        explicit Probe(glm::vec3 position);
        glm::vec3 get_position();
        void bake_geo(Shader& shader);
        void light();
        void cleanup();
        uint32_t get_albedo_handle() const;
        uint32_t get_normal_handle() const;
        uint32_t get_position_handle() const;
        uint32_t get_depth_handle() const;
        uint32_t get_orm_handle() const;
        uint64_t get_albedo_bindless() const;
        uint64_t get_normal_bindless() const;
        uint64_t get_position_bindless() const;
        uint64_t get_depth_bindless() const;
        uint64_t get_orm_bindless() const;
    private:
        glm::vec3 _position;
        uint32_t _albedo_handle = 0;
        uint32_t _normal_handle = 0;
        uint32_t _position_handle = 0;
        uint32_t _depth_handle = 0;
        uint32_t _orm_handle = 0;
        uint64_t _albedo_bindless = 0;
        uint64_t _normal_bindless = 0;
        uint64_t _position_bindless = 0;
        uint64_t _depth_bindless = 0;
        uint64_t _orm_bindless = 0;
    };
}
