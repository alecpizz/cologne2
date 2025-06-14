#pragma once
#include <engine/scene/Components.h>

namespace cologne
{
    class Editor
    {
        friend class Engine;

    public:
        ~Editor();

        void clear();

        void present();

        void add_float_entry(const char *name, float &value);

        void add_int_entry(const char *name, int &value);

        void add_vec3_entry(const char *name, glm::vec3 &value);

        void add_image_entry(const char *name, uint32_t value, const glm::vec2 &image_size);

        void add_bool_entry(const char *name, bool &value);

        void add_button(const char *name, std::function<void()> action);

        Editor(Editor &&) = delete;

        Editor(const Editor &) = delete;

        Editor &operator=(Editor &&) = delete;

        Editor &operator=(const Editor &) = delete;

        static bool in_edit_mode();

        static void toggle_edit_mode(bool b);

        static uint32_t get_viewport_width();

        static uint32_t get_viewport_height();

    private:
        Editor();

        void build();

        static void build_transform_entry(TransformComponent &tr);
    };
}
