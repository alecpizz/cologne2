#pragma once

namespace cologne
{
    class Texture
    {
    public:
        explicit Texture(const char *texture_path);
        Texture(unsigned char* data, uint32_t width, uint32_t height);
        Texture(uint32_t handle, uint32_t width, uint32_t height, uint32_t channels);
        Texture();

        ~Texture();

        uint32_t get_width() const;

        uint32_t get_height() const;

        uint32_t get_channels() const;

        uint32_t get_handle() const;

        void bind(uint8_t index) const;

        bool is_valid() const;

    private:
        uint32_t _handle = 0;
        uint32_t _width = 0;
        uint32_t _height = 0;
        uint32_t _channels = 0;
    };
}
