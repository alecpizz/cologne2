#pragma once
#include <shared_mutex>
#include <engine/scene/Components.h>
#include <engine/scene/Entity.h>

namespace cologne
{
    struct LogRecord
    {
        std::string message;
        ImVec4 color;
    };

    class Editor
    {
        friend class Engine;
    public:
        ~Editor();

        void present(float dt);

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

        static uint32_t get_viewport_width();

        static uint32_t get_viewport_height();

        static void initialize_reflection_editor();
        Editor();
        static void submit_log_record(LogRecord record);
    private:
        void build_main_window();

        void handle_hotkeys();

        void build_main_menu_bar();

        void build_asset_browser();

        void build_scene_graph();

        void build_properties_panel();

        void build_game_overlay();

        void build_game_view(float dt);

        void build_console();

        void build_images_window();

        void draw_entity_node(Entity entity);

        const char *_move_sound = ASSETS_PATH "sounds/menus/move.wav";
        const char *_accept_sound = ASSETS_PATH "sounds/menus/accept.wav";
        const char *_cancel_sound = ASSETS_PATH "sounds/menus/cancel.wav";
        uint32_t _global_window_flags = 0;
        bool _mouse_captured = false;
        bool _image_window_active = false;
        inline static bool _was_game_mode;
#define DEFAULT_WIDTH 1600
#define DEFAULT_HEIGHT 900
        inline static ImVec2 _prev_viewport_size;
        static std::vector<LogRecord> _records;
        static std::shared_timed_mutex _records_mutex;
        static void build_transform_entry(TransformComponent &tr);

        Entity _selected_entity = {};
    };
}
