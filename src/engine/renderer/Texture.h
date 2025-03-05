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

        void set_path(const char* path) const;

        TextureType get_type() const;

        void set_texture_type(TextureType type) const;

        void use(uint8_t index) const;

    private:
        struct Impl;
        Impl *_impl;
    };
}
