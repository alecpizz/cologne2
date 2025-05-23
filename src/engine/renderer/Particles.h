//
// Created by alecpizz on 5/23/2025.
//
#pragma once
#include "Shader.h"

namespace cologne
{
    class Particles
    {
    public:
        Particles();
        ~Particles();
        void init(const std::string& comp_path, uint32_t count);
        void cleanup();
        void simulate();
        void render();
    private:
        void calculate_positions(std::vector<glm::vec4>& positions);
        std::shared_ptr<Shader> _comp_shader = nullptr;
        uint32_t _total_particle_count = 0;
        uint32_t _particle_x, _particle_y, _particle_z;
        uint32_t _position_buffer, _velocity_buffer;
        uint32_t _vao, _vbo;
    };
}
