//
// Created by alecp on 3/16/2025.
//

#include "HDRTexture.h"

namespace goon
{
    HDRTexture::HDRTexture(const char *texture_path)
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, channels;
        float* data = stbi_loadf(texture_path, &width, &height, &channels, 0);
        if (!data)
        {
            LOG_ERROR("Failed to load hdr texture %s", texture_path);
        }
        else
        {
            glGenTextures(1, &_handle);
            glBindTexture(GL_TEXTURE_2D, _handle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            _width = width;
            _height = height;
            _channels = channels;
        }
        stbi_image_free(data);
    }

    HDRTexture::HDRTexture(unsigned char *data, uint32_t width, uint32_t height)
    {
    }

    HDRTexture::HDRTexture()
    {
    }

    HDRTexture::~HDRTexture()
    {
    }

    uint32_t HDRTexture::get_width() const
    {
        return _width;
    }

    uint32_t HDRTexture::get_height() const
    {
        return _height;
    }

    uint32_t HDRTexture::get_channels() const
    {
        return _channels;
    }

    uint32_t HDRTexture::get_handle() const
    {
        return _handle;
    }

    void HDRTexture::bind(uint8_t index) const
    {
        if (_handle == 0)
        {
            return;
        }
        glBindTextureUnit(index, _handle);
    }

    bool HDRTexture::is_valid() const
    {
        return _handle != 0;
    }
}
