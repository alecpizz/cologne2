#pragma once

namespace goon
{
    enum class TextureType
    {
        NONE,
        DIFFUSE,
        SPECULAR
    };
    class Texture
    {
    public:
        explicit Texture(const char *texture_path);

        ~Texture();

        uint32_t get_width() const;

        uint32_t get_height() const;

        uint32_t get_channels() const;

        uint32_t get_handle() const;

        const char* get_path() const;

        void set_path(const char* path);

        TextureType get_type() const;

        void set_texture_type(TextureType type);

        void use(uint8_t index) const;

    private:
        uint32_t _handle = 0;
        uint32_t _width = 0;
        uint32_t _height = 0;
        uint32_t _channels = 0;
        const char* _path = nullptr;
        TextureType _type = TextureType::NONE;
    };
}
