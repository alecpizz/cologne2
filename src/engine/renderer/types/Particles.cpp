//
// Created by alecpizz on 5/23/2025.
//

#include "Particles.h"
#include <engine/core/Engine.h>
#include <engine/core/Time.h>

#include "Shader.h"


namespace cologne
{
    void Particles::calculate_positions(std::vector<glm::vec4>& positions, std::vector<glm::vec4>& velocities)
    {
        auto bound_min = _bounds.min;
        auto bound_max = _bounds.max;

        float range_x = bound_min.x - bound_max.x;
        float range_y = bound_min.y - bound_max.y;
        float range_z = bound_min.z - bound_max.z;



        glm::vec4 p = glm::vec4(0.0f, 0.0, 0.0, 1.0f);
        glm::vec4 v = glm::vec4(0.0f);

        int idx = 0;
        for (uint32_t x = 0; x < _particle_x; x++)
        {
            for (uint32_t y = 0; y < _particle_y; y++)
            {
                for (uint32_t z = 0; z < _particle_z; z++)
                {
                    float x_pos = bound_min.x + glm::linearRand(0.0f, 1.0f) * range_x;
                    float y_pos = bound_min.y + glm::linearRand(0.0f, 1.0f) * range_y;
                    float z_pos = bound_min.z + glm::linearRand(0.0f, 1.0f) * range_z;
                    p.x = x_pos;
                    p.y = y_pos;
                    p.z = z_pos;
                    p.w = 1.0f;
                    v.x = glm::linearRand(-0.01f, 0.01f);
                    v.y = glm::linearRand(-0.01f, 0.01f);
                    v.z = glm::linearRand(-0.01f, 0.01f);
                    positions[idx] = p;
                    velocities[idx] = v;
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

    void Particles::init(AABB bounds, uint32_t count)
    {
        _bounds = bounds;
        std::vector<glm::vec4> positions;
        positions.resize(count * count * count);
        _total_particle_count = count * count * count;
        _particle_x = _particle_y = _particle_z = count;
        std::vector<glm::vec4> velocities (positions.size(), glm::vec4(0.0f));
        calculate_positions(positions, velocities);

        glCreateBuffers(1, &_position_buffer);
        glCreateBuffers(1, &_velocity_buffer);

        uint32_t buffer_size = positions.size() * sizeof(glm::vec4);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _position_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, positions.data(), GL_DYNAMIC_DRAW);


        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _velocity_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, velocities.data(), GL_DYNAMIC_COPY);

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        glBindVertexArray(0);
    }

    void Particles::simulate()
    {
        auto shader = Engine::get_renderer()->get_shader_by_name("particle_sim");
        if (!shader)
        {
            return;
        }
        shader->bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _position_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _velocity_buffer);
        shader->set_vec3("bounds_min", (_bounds.min));
        shader->set_vec3("bounds_max", (_bounds.max));
        shader->set_int("total_particle_count", _total_particle_count);
        static float time = 0.0f;
        time += Time::DeltaTime;
        shader->set_float("delta_time", Time::DeltaTime);
        shader->set_float("time", time);
        shader->dispatch((_total_particle_count + 127) / 128, 1, 1);
        shader->wait(GL_SHADER_STORAGE_BARRIER_BIT);
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
