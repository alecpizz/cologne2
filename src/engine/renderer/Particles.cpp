//
// Created by alecpizz on 5/23/2025.
//

#include "Particles.h"

#include "Shader.h"

namespace cologne
{
    void Particles::calculate_positions(std::vector<glm::vec4>& positions)
    {
        glm::vec4 p = glm::vec4(0.0f, 0.0, 0.0, 1.0f);
        float dx = 2.0 / _particle_x;
        float dy = 2.0 / _particle_y;
        float dz = 2.0 / _particle_z;

        glm::mat4 translation = glm::mat4(1.0);
        translation = glm::translate(translation, glm::vec3(-1.0f));

        int idx = 0;
        for (uint32_t x = 0; x < _particle_x; x++)
        {
            for (uint32_t y = 0; y < _particle_y; y++)
            {
                for (uint32_t z = 0; z < _particle_z; z++)
                {
                    p.x = dx * x;
                    p.y = dy * y;
                    p.z = dz * z;
                    p.w = 1.0f;
                    p = translation * p;
                    positions[idx] = p;
                    idx++;
                }
            }
        }

    }

    Particles::Particles(): _particle_x(0), _particle_y(0), _particle_z(0), _position_buffer(0), _velocity_buffer(0),
                            _vao(0), _vbo(0)
    {
    }

    Particles::~Particles()
    {
        LOG_INFO("CLEANING UP PARTICLES!");
    }

    void Particles::init(const std::string &comp_path, uint32_t count)
    {
        _comp_shader = std::make_shared<Shader>(comp_path.c_str());
        std::vector<glm::vec4> positions;
        positions.resize(count * count * count);
        _total_particle_count = count * count * count;
        _particle_x = _particle_y = _particle_z = count;
        calculate_positions(positions);

        glCreateBuffers(1, &_position_buffer);
        glCreateBuffers(1, &_velocity_buffer);

        uint32_t buffer_size = positions.size() * sizeof(glm::vec4);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _position_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, positions.data(), GL_DYNAMIC_DRAW);

        std::vector<glm::vec4> velocities (positions.size(), glm::vec4(0.0f));

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _velocity_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, velocities.data(), GL_DYNAMIC_COPY);

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        glBindVertexArray(0);
    }

    void Particles::simulate()
    {
        _comp_shader->bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _position_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _velocity_buffer);
        _comp_shader->dispatch(_total_particle_count, 1, 1);
        _comp_shader->wait(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
    }


    void Particles::render()
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _position_buffer);

        glBindVertexArray(_vao);
        glEnable(GL_PROGRAM_POINT_SIZE);

        glDrawArraysInstanced(GL_POINTS, 0, 1, _total_particle_count);
        glBindVertexArray(0);

        glDisable(GL_PROGRAM_POINT_SIZE);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    }
}
