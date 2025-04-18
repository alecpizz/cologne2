#pragma once

namespace cologne
{
    class DebugUI
    {
        friend class Engine;

    public:
        ~DebugUI();

        void clear();

        void present();

        void add_float_entry(const char *name, float &value);

        void add_int_entry(const char *name, int &value);

        void add_vec3_entry(const char *name, glm::vec3 &value);

        void add_image_entry(const char *name, uint32_t value, const glm::vec2 &image_size);

        void add_bool_entry(const char *name, bool &value);

        DebugUI(DebugUI &&) = delete;

        DebugUI(const DebugUI &) = delete;

        DebugUI &operator=(DebugUI &&) = delete;

        DebugUI &operator=(const DebugUI &) = delete;

    private:
        DebugUI();

        struct Impl;
        Impl *_impl;
    };
}
