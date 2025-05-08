#pragma once

namespace cologne
{
    class Renderer;
    class TextRenderer
    {
        friend class Renderer;
    public:
        ~TextRenderer();

        TextRenderer(TextRenderer &&) = delete;

        TextRenderer(const TextRenderer &) = delete;

        TextRenderer &operator=(TextRenderer &&) = delete;

        TextRenderer &operator=(const TextRenderer &) = delete;

        void draw_text(const char* text, glm::vec3 position, glm::vec4 color, float size);
        void present();
    private:
        explicit TextRenderer(const char* font_path);
    };
}
