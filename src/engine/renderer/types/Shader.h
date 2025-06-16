//
// Created by alecpizz on 3/1/2025.
//

#pragma once

namespace cologne
{
    class Shader
    {
    public:
        Shader() = default;

        explicit Shader(const std::string &comp_path);

        Shader(const std::string &vert_path, const std::string &frag_path);

        Shader(const std::string &vert_path, const std::string &frag_path, const std::string &geom_path);

        ~Shader();

        uint32_t get_handle() const;

        void bind() const;

        void dispatch(uint32_t work_size_x, uint32_t work_size_y, uint32_t work_size_z);

        void wait();

        void wait(uint32_t barriers);

        void set_bool(const std::string& name, bool value);

        void set_int(const std::string& name, int32_t value);

        void set_vec3(const std::string &name, glm::vec3 value);

        void set_vec2(const std::string &name, glm::vec2 value);

        void set_vec4(const std::string &name, glm::vec4 value);

        void set_mat4(const std::string &name, glm::mat4 value);

        void set_mat4(const std::string &name, const std::vector<glm::mat4> &value);

        void set_float(const std::string &name, float value);

        void set_uint(const std::string &name, uint32_t id);

    private:
        void compile(const std::string &comp_path);

        void compile(const std::string &vert_path, const std::string &frag_path,
                     const std::string &geo_path = std::string());

        uint32_t add_shader(const std::string &shader_path, const GLenum shader_type) const;

        uint32_t _program = 0;
        std::string _name = std::string();
        std::unordered_map<std::string, int32_t> _uniforms;
        bool _linked = false;
    };
}
