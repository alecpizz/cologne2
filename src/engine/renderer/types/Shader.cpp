//
// Created by alecpizz on 3/1/2025.
//

#include "Shader.h"
#include <fstream>

namespace cologne
{
    Shader::Shader(const std::string&comp_path)
    {
        compile(comp_path);
    }

    Shader::Shader(const char* vert_path, const char* frag_path)
    {
        compile(vert_path, frag_path, std::string());
    }

    Shader::Shader(const std::string&vert_path, const std::string&frag_path, const std::string&geom_path)
    {
        compile(vert_path, frag_path, geom_path);
    }

    Shader::~Shader()
    {
        // if (_program != 0)
        // {
        //     LOG_INFO("DELETING SHADER %s", _name.c_str());
        //     glDeleteProgram(_program);
        // }
    }

    uint32_t Shader::get_handle() const
    {
        return _program;
    }

    void Shader::bind() const
    {
        if (!_linked)
        {
            return;
        }
        glUseProgram(_program);
    }

    void Shader::dispatch(uint32_t work_size_x, uint32_t work_size_y, uint32_t work_size_z)
    {
        if (!_linked)
        {
            return;
        }
        glDispatchCompute(work_size_x, work_size_y, work_size_z);
    }

    void Shader::wait()
    {
        if (!_linked)
        {
            return;
        }
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void Shader::wait(uint32_t barriers)
    {
        glMemoryBarrier(barriers);
    }

    void Shader::set_bool(const std::string&name, const bool value)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniform1i(_program, _uniforms[name], static_cast<int>(value));
    }

    void Shader::set_int(const std::string&name, int32_t value)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniform1i(_program, _uniforms[name], value);
    }

    void Shader::set_vec3(const std::string &name, glm::vec3 value)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniform3fv(_program, _uniforms[name], 1, glm::value_ptr(value));
    }

    void Shader::set_vec2(const std::string &name, glm::vec2 value)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniform2fv(_program, _uniforms[name], 1, glm::value_ptr(value));
    }

    void Shader::set_vec4(const std::string&name, glm::vec4 value)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniform4fv(_program, _uniforms[name], 1, glm::value_ptr(value));
    }

    void Shader::set_mat4(const std::string& name, glm::mat4 value)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniformMatrix4fv(_program, _uniforms[name], 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::set_mat4(const std::string &name, const std::vector<glm::mat4> &value)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniformMatrix4fv(_program, _uniforms[name], value.size(), GL_FALSE, glm::value_ptr(value[0]));
    }

    void Shader::set_float(const std::string&name, const float value)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniform1f(_program, _uniforms[name], value);
    }

    void Shader::set_uint(const std::string &name, const uint32_t id)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniform1ui(_program, _uniforms[name], id);
    }

    void Shader::set_ivec2(const std::string &name, std::vector<glm::ivec2> vecs)
    {
        if (!_linked)
        {
            return;
        }
        if (!_uniforms.contains(name))
        {
            _uniforms[name] = glGetUniformLocation(_program, name.c_str());
        }
        glProgramUniform2iv(_program, _uniforms[name], vecs.size(), glm::value_ptr(vecs[0]));
    }

    void Shader::cleanup()
    {
        if (_program != 0)
        {
            glDeleteProgram(_program);
        }
    }

    void Shader::compile(const std::string &comp_path)
    {
        _program = glCreateProgram();

        if (_program == 0)
        {
            LOG_ERROR("Failed to create program");
            return;
        }
        uint32_t comp_shader = add_shader(comp_path, GL_COMPUTE_SHADER);
        if (comp_shader != 0)
        {
            glAttachShader(_program, comp_shader);
        }

        glLinkProgram(_program);
        int32_t success;
        glGetProgramiv(_program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(_program, 512, nullptr, infoLog);
            LOG_ERROR("Failed to link program %s", infoLog);
            glDeleteShader(comp_shader);
            return;
        }
        glDeleteShader(comp_shader);
        _linked = true;
    }

    void Shader::compile(const std::string &vert_path, const std::string &frag_path, const std::string &geo_path)
    {
        _program = glCreateProgram();

        if (_program == 0)
        {
            LOG_ERROR("Failed to create program");
            return;
        }
        const uint32_t vertex_shader = add_shader(vert_path, GL_VERTEX_SHADER);
        const uint32_t fragment_shader = add_shader(frag_path, GL_FRAGMENT_SHADER);
        const uint32_t geometry_shader = add_shader(geo_path, GL_GEOMETRY_SHADER);
        if (vertex_shader != 0)
        {
            glAttachShader(_program, vertex_shader);
        }
        if (fragment_shader != 0)
        {
            glAttachShader(_program, fragment_shader);
        }
        if (geometry_shader != 0)
        {
            glAttachShader(_program, geometry_shader);
        }

        glLinkProgram(_program);
        int32_t success;
        glGetProgramiv(_program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(_program, 512, nullptr, infoLog);
            LOG_ERROR("Failed to link program %s", infoLog);
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            glDeleteShader(geometry_shader);
            return;
        }
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteShader(geometry_shader);
        _linked = true;
    }

    uint32_t Shader::add_shader(const std::string &shader_path, const GLenum shader_type) const
    {
        if (shader_path.empty())
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
            LOG_ERROR("Failed to read file %s %s", shader_path.c_str(), e.what());
        }
        uint32_t shader = glCreateShader(shader_type);
        const char* shader_code = code.c_str();
        glShaderSource(shader, 1, &shader_code, nullptr);
        glCompileShader(shader);
        int32_t success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char info_log[512];
            glGetShaderInfoLog(shader, 512, nullptr, info_log);
            LOG_ERROR("Failed to compile shader %s with reason: %s", shader_path.c_str(), info_log);
        }
        return shader;
    }
}
