//
// Created by alecpizz on 3/1/2025.
//

#include "Texture.h"

namespace goon
{

    Texture::Texture(const char *texture_path)
    {
        set_path(texture_path);
        glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        stbi_set_flip_vertically_on_load(true);
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
            glTextureStorage2D(_handle, 1, GL_RGBA8, _width, _height);
            glTextureSubImage2D(_handle, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateTextureMipmap(_handle);
        }
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::Texture(void *data, const char *texture_path, uint32_t width, uint32_t height, uint32_t channels)
    {
        set_path(texture_path);
        glGenTextures(1, &_handle);
        glBindTexture(GL_TEXTURE_2D, _handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (!data)
        {
            glDeleteTextures(1, &_handle);
        }
        else
        {
            _width = width;
            _height = height;
            _channels = channels;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                         channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::Texture()
    {
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &_handle);
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

    const char *Texture::get_path() const
    {
        return _path;
    }

    void Texture::set_path(const char *path)
    {
        _path = std::string(path).c_str();
    }

    TextureType Texture::get_type() const
    {
        return _type;
    }

    void Texture::set_texture_type(const TextureType type)
    {
        _type = type;
    }

    void Texture::bind(const uint8_t index) const
    {
        glBindTextureUnit(index, _handle);
    }
}
