//
// Created by alecpizz on 3/1/2025.
//


#include "Texture.h"

#include <stb_dxt/stb_dxt.h>
#include <fstream>
#include <engine/util/FileUtil.h>

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
        // int new_width, new_height, new_channels;
        _width = width;
        _height = height;
        if (height == 0)
        {
            _data = std::vector<unsigned char>(data, data + width);
        } else
        {
            _data = std::vector<unsigned char>(data, data + (width * height));
        }
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

    uint64_t Texture::get_bindless_handle() const
    {
        return _bindless_handle;
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

    bool Texture::contains_data() const
    {
        return !_data.empty();
    }

    void Texture::load()
    {
        if (_data.empty())
        {
            return;
        }
        stbi_set_flip_vertically_on_load(false);
        int new_width = 0, new_height = 0, new_channels = 0;
        stbi_uc *img_data =
            stbi_load_from_memory(_data.data(), static_cast<int>(_data.size()),
                &new_width, &new_height, &new_channels, 0);
        _width = new_width;
        _height = new_height;
        _channels = new_channels;

        glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int32_t format = GL_RGB;
        int32_t format_internal = GL_RGBA8;
        if (_channels == 1)
        {
            format = GL_RED;
            format_internal = GL_R8;
        } else if (_channels == 3)
        {
            format = GL_RGB;
            format_internal = GL_RGB8;
        } else if (_channels == 4)
        {
            format = GL_RGBA;
            format_internal = GL_RGBA8;
        }
        int32_t mips = floor(log2(std::max(_width, _height))) + 1;
        glTextureStorage2D(_handle, mips, format_internal, _width, _height);
        glTextureSubImage2D(_handle, 0, 0, 0, _width, _height, format, GL_UNSIGNED_BYTE,
                            img_data);
        glGenerateTextureMipmap(_handle);
        stbi_image_free(img_data);
        _data.clear();
        _data.shrink_to_fit();
    }

    void Texture::make_resident()
    {
        if (_handle == 0)
        {
            return;
        }
        if (_is_resident)
        {
            return;
        }
        _bindless_handle = glGetTextureHandleARB(_handle);
        glMakeTextureHandleResidentARB(_bindless_handle);
        _is_resident = true;
    }

    void Texture::make_non_resident()
    {
        if (_handle == 0)
        {
            return;
        }
        if (!_is_resident)
        {
            return;
        }
        glMakeTextureHandleNonResidentARB(_bindless_handle);
        _is_resident = false;
    }

    void Texture::export_to_compressed(const char* path) const
    {
        if (_data.empty())
        {
            LOG_ERROR("No data to compress!");
            return;
        }
        int new_width;
        int new_height;
        int new_channels;
        stbi_uc *img_data =
           stbi_load_from_memory(_data.data(), static_cast<int>(_data.size()),
               &new_width, &new_height, &new_channels, 4);
        if (new_width % 4 != 0 || new_height % 4 != 0)
        {
            //TODO: handle this
            LOG_ERROR("Can't export texture because it isn't a multiple of four.");
            stbi_image_free(img_data);
            return;
        }

        int compressed_size = (new_width * new_height) / 2;
        std::vector<uint8_t> compressed_data(compressed_size);
        for (int y = 0; y < new_height; y += 4)
        {
            for (int x = 0; x < new_width; x += 4)
            {
                int block_x = x / 4;
                int block_y = y / 4;
                int blocks_per_row = new_width / 4;
                uint8_t* dest_block = &compressed_data[(block_y * blocks_per_row + block_x) * 8];

                uint8_t source_block[16 * 4];

                for (int row_in_block = 0; row_in_block < 4; row_in_block++)
                {
                    uint8_t* source_row = img_data + ((y + row_in_block) * new_width + x) * 4;

                    uint8_t* dest_row = img_data + (row_in_block * 4 * 4);

                    memcpy(dest_row, source_row, 16);
                }

                stb_compress_dxt_block(dest_block, source_block, 0, STB_DXT_HIGHQUAL);
            }
        }

        FileUtil::create_directory_recursive(path);
        std::ofstream outfile (path, std::ios::binary);
        if (outfile.fail())
        {
            LOG_ERROR("Couldn't open output file %s %s", path, strerror(errno));
            stbi_image_free(img_data);
            return;
        }

        outfile.write(reinterpret_cast<const char*>(compressed_data.data()), compressed_data.size());
        outfile.close();
    }
}
