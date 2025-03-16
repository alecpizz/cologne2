#pragma once

namespace goon
{
    enum class TextureType
    {
        NONE = -1,
        ALBEDO = 0,
        NORMAL = 1,
        AO = 2,
        ROUGHNESS = 3,
        METALLIC = 4
    };
    class Texture
    {
    public:
        explicit Texture(const char *texture_path);
        Texture(unsigned char* data, uint32_t width, uint32_t height);
        Texture();

        ~Texture();

        uint32_t get_width() const;

        uint32_t get_height() const;

        uint32_t get_channels() const;

        uint32_t get_handle() const;

        TextureType get_type() const;

        void set_texture_type(TextureType type);

        void bind(uint8_t index) const;

        bool is_valid() const;

    private:
        uint32_t _handle = 0;
        uint32_t _width = 0;
        uint32_t _height = 0;
        uint32_t _channels = 0;
        TextureType _type = TextureType::NONE;
    };
}
