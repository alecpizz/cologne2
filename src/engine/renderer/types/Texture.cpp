//
// Created by alecpizz on 3/1/2025.
//


#include "Texture.h"

#include <fstream>
#include <engine/util/FileUtil.h>
#include <filesystem>
#include <squish/squish.h>

namespace cologne
{
#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1"

    struct DDS_PIXELFORMAT
    {
        uint32_t dwSize;
        uint32_t dwFlags;
        uint32_t dwFourCC;
        uint32_t dwRGBBitCount;
        uint32_t dwRBitMask;
        uint32_t dwGBitMask;
        uint32_t dwBBitMask;
        uint32_t dwABitMask;
    };

    struct DDS_HEADER
    {
        uint32_t dwSize;
        uint32_t dwFlags;
        uint32_t dwHeight;
        uint32_t dwWidth;
        uint32_t dwPitchOrLinearSize;
        uint32_t dwDepth;
        uint32_t dwMipMapCount;
        uint32_t dwReserved1[11];
        DDS_PIXELFORMAT ddspf;
        uint32_t dwCaps;
        uint32_t dwCaps2;
        uint32_t dwCaps3;
        uint32_t dwCaps4;
        uint32_t dwReserved2;
    };


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
        }
        else
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
        }
        else
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
        }
        else if (_channels == 3)
        {
            format = GL_RGB;
            format_internal = GL_RGB8;
        }
        else if (_channels == 4)
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

    void Texture::load_compressed()
    {
        if (_path.empty())
        {
            return;
        }
        std::filesystem::path f_path(_path);
        if (_path.length() > 512)
        {
            LOG_WARN("PATH TOO LONG, SKIPPING");
            return;
        }
        if (!std::filesystem::exists(f_path))
        {
            LOG_ERROR("No file found at %s", _path.c_str());
            return;
        }

        if (!f_path.has_extension())
        {
            _data.clear();
            _data.shrink_to_fit();
            return;
        }

        if (f_path.extension() != ".ctext")
        {
            LOG_ERROR("INVALID TEXTURE EXTENSION! %d", f_path.extension().c_str());
            _data.clear();
            _data.shrink_to_fit();
            return;
        }

        std::ifstream file(_path, std::ios::binary);
        if (file.fail())
        {
            LOG_ERROR("Couldn't open import file %s %s", _path.c_str(), strerror(errno));
            return;
        }

        // outfile.write("DDS ", 4);
        // outfile.write(reinterpret_cast<const char*>(&header), sizeof(header));
        // outfile.write(reinterpret_cast<const char*>(compressed_data.data()), compressed_data.size());
        // outfile.close();
        char sign[4];
        file.read(reinterpret_cast<char *>(&sign), 4);
        if (!strcmp(sign, "DDS "))
        {
            LOG_ERROR("File not DDS!, got: %s", sign);
            return;
        }

        DDS_HEADER header = {};
        file.read(reinterpret_cast<char *>(&header), sizeof(header));
        int new_width = header.dwWidth;
        int new_height = header.dwHeight;
        int channels = 4;
        int compressed_size = header.dwPitchOrLinearSize;
        std::vector<uint8_t> compressed_data(compressed_size);
        file.read(reinterpret_cast<char *>(compressed_data.data()), compressed_data.size());
        file.close();

        _width = new_width;
        _height = new_height;
        _channels = channels;

        glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int32_t format = GL_RGBA;
        int32_t format_internal = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

        int32_t mips = floor(log2(std::max(_width, _height))) + 1;
        glTextureStorage2D(_handle, mips, format_internal, _width, _height);
        glCompressedTextureSubImage2D(_handle, 0, 0, 0, _width, _height, format_internal, compressed_size,
                                      compressed_data.data());
        glGenerateTextureMipmap(_handle);
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

    void Texture::export_to_compressed(const char *path) const
    {
        if (_data.empty())
        {
            LOG_ERROR("No data to compress!");
            return;
        }
        int new_width;
        int new_height;
        int new_channels;
        stbi_set_flip_vertically_on_load(false);
        stbi_uc *img_data = stbi_load_from_memory(_data.data(), static_cast<int>(_data.size()),
                                      &new_width, &new_height, &new_channels, 4);
        FileUtil::create_directory_recursive(path);
        std::ofstream outfile(path, std::ios::binary);
        if (outfile.fail())
        {
            LOG_ERROR("Couldn't open output file %s %s", path, strerror(errno));
            return;
        }


        int cFlags = squish::kDxt1;
        int cSize = squish::GetStorageRequirements(new_width, new_height, cFlags);
        DDS_HEADER header = {};
        header.dwSize = 124;
        header.dwFlags = 0x1 | 0x2 | 0x4 | 0x1000 | 0x80000; // CAPS, HEIGHT, WIDTH, PIXELFORMAT, LINEARSIZE
        header.dwHeight = new_height;
        header.dwWidth = new_width;
        header.dwPitchOrLinearSize = cSize;
        header.dwDepth = 0;
        header.dwMipMapCount = 0;
        header.ddspf.dwSize = 32;
        header.ddspf.dwFlags = 0x4; // FOURCC
        header.ddspf.dwFourCC = FOURCC_DXT1;
        header.dwCaps = 0x1000; // TEXTURE

        outfile.write("DDS ", 4);
        outfile.write(reinterpret_cast<const char *>(&header), sizeof(header));

        std::vector<uint8_t> pixels(cSize);
        squish::CompressImage(img_data, new_width, new_height, new_width * 4, pixels.data(), cFlags);

        outfile.write(reinterpret_cast<const char *>(pixels.data()), pixels.size());

        stbi_image_free(img_data);
        outfile.close();
    }
}
