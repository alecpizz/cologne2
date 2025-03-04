//
// Created by alecpizz on 3/1/2025.
//

#include "Texture.h"

namespace goon
{
    struct Texture::Impl
    {
        uint32_t handle = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channels = 0;
        std::string path;
        TextureType type = TextureType::NONE;

        void init(const std::string& path)
        {
            this->path = path;
            glGenTextures(1, &handle);
            glBindTexture(GL_TEXTURE_2D, handle);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


            stbi_set_flip_vertically_on_load(true);
            int width, height, channels;
            auto data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!data)
            {
                LOG_ERROR("Failed to load texture from %s", path);
                glDeleteTextures(1, &handle);
            }
            else
            {
                this->width = width;
                this->height = height;
                this->channels = channels;
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                    channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            stbi_image_free(data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    };

    Texture::Texture(const char *texture_path)
    {
        _impl = new Impl();
        _impl->init(texture_path);
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &_impl->handle);
        delete _impl;
    }

    uint32_t Texture::get_width() const
    {
        return _impl->width;
    }

    uint32_t Texture::get_height() const
    {
        return _impl->height;
    }

    uint32_t Texture::get_channels() const
    {
        return _impl->channels;
    }

    uint32_t Texture::get_handle() const
    {
        return _impl->handle;
    }

    const char * Texture::get_path() const
    {
        return _impl->path.c_str();
    }

    void Texture::set_path(const char *path) const
    {
        _impl->path = std::string(path);
    }

    TextureType Texture::get_type() const
    {
        return _impl->type;
    }

    void Texture::set_texture_type(const TextureType type) const
    {
        _impl->type = type;
    }

    void Texture::use(const uint8_t index) const
    {
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, _impl->handle);
    }
}
