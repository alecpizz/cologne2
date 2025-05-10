//
// Created by alecpizz on 3/1/2025.
//

#include "Texture.h"

namespace cologne
{
    Texture::Texture(const char *texture_path)
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        int width, height, channels;
        auto data = stbi_load(texture_path, &width, &height, &channels, STBI_rgb_alpha);
        if (!data)
        {
            LOG_ERROR("Failed to load texture from");
            LOG_ERROR(texture_path);
            glDeleteTextures(1, &_handle);
        } else
        {
            _width = width;
            _height = height;
            _channels = channels;
            int32_t mips = 1 + log2(std::max(_width, _height));
            LOG_INFO("GENERATING MIPS %d", mips);
            glTextureStorage2D(_handle, mips, GL_RGBA8, _width, _height);
            glTextureSubImage2D(_handle, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateTextureMipmap(_handle);
        }
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::Texture(unsigned char *data, uint32_t width, uint32_t height)
    {
        int new_width, new_height, new_channels;
        stbi_uc *image_data;
        if (height == 0)
        {
            image_data = stbi_load_from_memory(data, width, &new_width, &new_height, &new_channels, 0);
        } else
        {
            image_data = stbi_load_from_memory(data, width * height, &new_width, &new_height, &new_channels, 0);
        }


        glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (!image_data)
        {
            LOG_ERROR("Failed to load texture from ptr");
            glDeleteTextures(1, &_handle);
        } else
        {
            _width = new_width;
            _height = new_height;
            _channels = new_channels;
            int32_t format = GL_RGB;
            int32_t format_internal = GL_RGBA8;
            if (new_channels == 1)
            {
                format = GL_RED;
                format_internal = GL_R8;
            } else if (new_channels == 3)
            {
                format = GL_RGB;
                format_internal = GL_RGB8;
            } else if (new_channels == 4)
            {
                format = GL_RGBA;
                format_internal = GL_RGBA8;
            }
            int32_t mips = ceil(log2(std::max(_width, _height))) + 1;
            glTextureStorage2D(_handle, mips, format_internal, _width, _height);
            glTextureSubImage2D(_handle, 0, 0, 0, _width, _height, format, GL_UNSIGNED_BYTE, image_data);
            glGenerateTextureMipmap(_handle);
        }
        stbi_image_free(image_data);
    }

    Texture::Texture(uint32_t handle, uint32_t width, uint32_t height, uint32_t channels)
    {
        _handle = handle;
        _width = width;
        _height = height;
        _channels = channels;
    }

    Texture::Texture()
    {
    }

    Texture::~Texture()
    {
        // glDeleteTextures(1, &_handle);
    }

    uint32_t Texture::get_width() const
    {
        return _width;
    }

    uint32_t Texture::get_height() const
    {
        return _height;
    }

    uint32_t Texture::get_channels() const
    {
        return _channels;
    }

    uint32_t Texture::get_handle() const
    {
        return _handle;
    }


    void Texture::bind(const uint8_t index) const
    {
        if (_handle == 0)
        {
            glBindTextureUnit(index, 0);
            return;
        }
        glBindTextureUnit(index, _handle);
    }

    bool Texture::is_valid() const
    {
        return _handle != 0;
    }
}
