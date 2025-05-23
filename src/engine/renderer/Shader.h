//
// Created by alecpizz on 3/1/2025.
//

#pragma once

namespace cologne
{
    class Shader
    {
    public:
        Shader(Shader &&) = delete;

        Shader(const Shader &) = delete;

        Shader &operator=(Shader &&) = delete;

        Shader &operator=(const Shader &) = delete;

        Shader(const char* comp_path);

        Shader(const char *vert_path, const char *frag_path);

        Shader(const char *vert_path, const char *frag_path, const char *geom_path);

        ~Shader();

        uint32_t get_handle() const;

        void bind() const;
        void dispatch(uint32_t work_size_x, uint32_t work_size_y, uint32_t work_size_z);
        void wait();
        void wait(uint32_t barriers);

        void set_bool(const char *name, bool value) const;

        void set_int(const char *name, int32_t value) const;

        void set_vec3(const char *name, const float *value) const;
        void set_vec2(const char *name, const float *value) const;

        void set_vec4(const char *name, const float *value) const;

        void set_mat4(const char *name, const float *value) const;

        void set_float(const char *name, float value) const;

    private:
        struct Impl;
        Impl *_impl;
    };
}
