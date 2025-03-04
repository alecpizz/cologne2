#pragma once

namespace goon
{
    class Texture
    {
    public:
        explicit Texture(const char *texture_path);

        ~Texture();

        uint32_t get_width() const;

        uint32_t get_height() const;

        uint32_t get_channels() const;

        uint32_t get_handle() const;

        void use(uint8_t index) const;

        Texture(const Texture &) = delete;

        Texture &operator=(const Texture &) = delete;

        Texture(Texture &&) = delete;

        Texture &operator=(Texture &&) = delete;

    private:
        struct Impl;
        Impl *_impl;
    };
}
