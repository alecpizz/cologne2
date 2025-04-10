//
// Created by alecpizz on 3/1/2025.
//

#include "Shader.h"
#include <fstream>

namespace cologne
{
    struct Shader::Impl
    {
        uint32_t program = 0;
        std::string name;
        std::unordered_map<std::string, int32_t> uniforms;

        void compile(const char *vertex_path, const char *fragment_path, const char *geometry_path)
        {
            program = glCreateProgram();

            if (program == 0)
            {
                LOG_ERROR("Failed to create program");
                return;
            }
            uint32_t vertex_shader = add_shader(vertex_path, GL_VERTEX_SHADER);
            uint32_t fragment_shader = add_shader(fragment_path, GL_FRAGMENT_SHADER);
            uint32_t geometry_shader = add_shader(geometry_path, GL_GEOMETRY_SHADER);
            if (vertex_shader != 0)
            {
                glAttachShader(program, vertex_shader);
            }
            if (fragment_shader != 0)
            {
                glAttachShader(program, fragment_shader);
            }
            if (geometry_shader != 0)
            {
                glAttachShader(program, geometry_shader);
            }

            glLinkProgram(program);
            int32_t success;
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (!success)
            {
                char infoLog[512];
                glGetProgramInfoLog(program, 512, nullptr, infoLog);
                LOG_ERROR("Failed to link program %s", infoLog);
            }
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            glDeleteShader(geometry_shader);
        }

        void compile(const char *comp_path)
        {
            program = glCreateProgram();

            if (program == 0)
            {
                LOG_ERROR("Failed to create program");
                return;
            }
            uint32_t comp_shader = add_shader(comp_path, GL_COMPUTE_SHADER);
            if (comp_shader != 0)
            {
                glAttachShader(program, comp_shader);
            }

            glLinkProgram(program);
            int32_t success;
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (!success)
            {
                char infoLog[512];
                glGetProgramInfoLog(program, 512, nullptr, infoLog);
                LOG_ERROR("Failed to link program %s", infoLog);
            }
            glDeleteShader(comp_shader);
        }

        uint32_t add_shader(const char *shader_path, const GLenum shader_type) const
        {
            if (shader_path == nullptr)
            {
                return 0;
            }
            std::string code;
            std::ifstream file;
            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try
            {
                file.open(shader_path);
                std::stringstream ss;
                ss << file.rdbuf();
                file.close();
                code = ss.str();
            } catch (const std::ifstream::failure &e)
            {
                LOG_ERROR("Failed to read file %s %s", shader_path, e.what());
            }
            uint32_t shader = glCreateShader(shader_type);
            const char *shader_code = code.c_str();
            glShaderSource(shader, 1, &shader_code, nullptr);
            glCompileShader(shader);
            int32_t success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                char info_log[512];
                glGetShaderInfoLog(shader, 512, nullptr, info_log);
                LOG_ERROR("Failed to compile shader %s with reason: %s", shader_path, info_log);
            }
            return shader;
        }
    };

    Shader::Shader(const char *comp_path)
    {
        _impl = new Impl();
        _impl->compile(comp_path);
    }

    Shader::Shader(const char *vert_path, const char *frag_path)
    {
        _impl = new Impl();
        _impl->compile(vert_path, frag_path, nullptr);
    }

    Shader::Shader(const char *vert_path, const char *frag_path, const char *geom_path)
    {
        _impl = new Impl();
        _impl->compile(vert_path, frag_path, geom_path);
    }

    Shader::~Shader()
    {
        glDeleteProgram(_impl->program);
        delete _impl;
    }

    uint32_t Shader::get_handle() const
    {
        return _impl->program;
    }

    void Shader::bind() const
    {
        glUseProgram(_impl->program);
    }

    void Shader::dispatch(uint32_t work_size_x, uint32_t work_size_y, uint32_t work_size_z)
    {
        glDispatchCompute(work_size_x, work_size_y, work_size_z);
    }

    void Shader::wait()
    {
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    void Shader::set_bool(const char *name, const bool value) const
    {
        if (!_impl->uniforms.contains(name))
        {
            _impl->uniforms[name] = glGetUniformLocation(_impl->program, name);
        }
        glUniform1i(_impl->uniforms[name], static_cast<int>(value));
    }

    void Shader::set_int(const char *name, int32_t value) const
    {
        if (!_impl->uniforms.contains(name))
        {
            _impl->uniforms[name] = glGetUniformLocation(_impl->program, name);
        }
        glUniform1i(_impl->uniforms[name], value);
    }

    void Shader::set_vec3(const char *name, const float *value) const
    {
        if (!_impl->uniforms.contains(name))
        {
            _impl->uniforms[name] = glGetUniformLocation(_impl->program, name);
        }
        glUniform3fv(_impl->uniforms[name], 1, value);
    }

    void Shader::set_vec4(const char *name, const float *value) const
    {
        if (!_impl->uniforms.contains(name))
        {
            _impl->uniforms[name] = glGetUniformLocation(_impl->program, name);
        }
        glUniform4fv(_impl->uniforms[name], 1, value);
    }

    void Shader::set_mat4(const char *name, const float *value) const
    {
        if (!_impl->uniforms.contains(name))
        {
            _impl->uniforms[name] = glGetUniformLocation(_impl->program, name);
        }
        glUniformMatrix4fv(_impl->uniforms[name], 1, GL_FALSE, value);
    }

    void Shader::set_float(const char *name, const float value) const
    {
        if (!_impl->uniforms.contains(name))
        {
            _impl->uniforms[name] = glGetUniformLocation(_impl->program, name);
        }
        glUniform1f(_impl->uniforms[name], value);
    }
}
