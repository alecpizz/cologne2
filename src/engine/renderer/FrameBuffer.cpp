//
// Created by alecpizz on 3/1/2025.
//

#include "FrameBuffer.h"

namespace cologne
{
    struct FrameBuffer::Impl
    {
        std::vector<ColorAttachment> color_attachments;

        static int32_t translate_internal_format_to_format(uint32_t format)
        {
            switch (format)
            {
                // Red channel formats
                case GL_R8:
                case GL_R8_SNORM:
                case GL_R16:
                case GL_R16_SNORM:
                case GL_R16F:
                case GL_R32F:
                    return GL_RED;
                case GL_R8UI:
                case GL_R8I:
                case GL_R16UI:
                case GL_R16I:
                case GL_R32UI:
                case GL_R32I:
                    return GL_RED_INTEGER;

                // Red-Green channel formats
                case GL_RG8:
                case GL_RG8_SNORM:
                case GL_RG16:
                case GL_RG16_SNORM:
                case GL_RG16F:
                case GL_RG32F:
                    return GL_RG;
                case GL_RG8UI:
                case GL_RG8I:
                case GL_RG16UI:
                case GL_RG16I:
                case GL_RG32UI:
                case GL_RG32I:
                    return GL_RG_INTEGER;

                // RGB channel formats
                case GL_RGB8:
                case GL_RGB8_SNORM:
                case GL_RGB16:
                case GL_RGB16_SNORM:
                case GL_RGB16F:
                case GL_RGB32F:
                case GL_SRGB8:
                    return GL_RGB;
                case GL_RGB8UI:
                case GL_RGB8I:
                case GL_RGB16UI:
                case GL_RGB16I:
                case GL_RGB32UI:
                case GL_RGB32I:
                    return GL_RGB_INTEGER;

                // RGBA channel formats
                case GL_RGBA8:
                case GL_RGBA8_SNORM:
                case GL_RGBA16:
                case GL_RGBA16_SNORM:
                case GL_RGBA16F:
                case GL_RGBA32F:
                case GL_SRGB8_ALPHA8:
                    return GL_RGBA;
                case GL_RGBA8UI:
                case GL_RGBA8I:
                case GL_RGBA16UI:
                case GL_RGBA16I:
                case GL_RGBA32UI:
                case GL_RGBA32I:
                    return GL_RGBA_INTEGER;

                // Special packed formats
                case GL_RGB10_A2:
                case GL_RGB10_A2UI:
                    return GL_RGBA;

                // Depth formats
                case GL_DEPTH_COMPONENT16:
                case GL_DEPTH_COMPONENT24:
                case GL_DEPTH_COMPONENT32F:
                    return GL_DEPTH_COMPONENT;

                // Depth-stencil formats
                case GL_DEPTH24_STENCIL8:
                case GL_DEPTH32F_STENCIL8:
                    return GL_DEPTH_STENCIL;

                default:
                    std::cout << "GLInternalFormatToGLFormat: Unsupported internal format\n";
                    return 0;
            }
        }

        static int32_t translate_internal_format_to_type(uint32_t format)
        {
            switch (format)
            {
                // 8-bit unsigned and signed integer formats
                case GL_R8UI: return GL_UNSIGNED_BYTE;
                case GL_R8I: return GL_BYTE;
                case GL_RG8UI: return GL_UNSIGNED_BYTE;
                case GL_RG8I: return GL_BYTE;
                case GL_RGBA8UI: return GL_UNSIGNED_BYTE;
                case GL_RGBA8I: return GL_BYTE;

                // 16-bit unsigned and signed integer formats
                case GL_R16UI: return GL_UNSIGNED_SHORT;
                case GL_R16I: return GL_SHORT;
                case GL_RG16UI: return GL_UNSIGNED_SHORT;
                case GL_RG16I: return GL_SHORT;
                case GL_RGBA16UI: return GL_UNSIGNED_SHORT;
                case GL_RGBA16I: return GL_SHORT;

                // 32-bit unsigned and signed integer formats
                case GL_R32UI: return GL_UNSIGNED_INT;
                case GL_R32I: return GL_INT;
                case GL_RG32UI: return GL_UNSIGNED_INT;
                case GL_RG32I: return GL_INT;
                case GL_RGBA32UI: return GL_UNSIGNED_INT;
                case GL_RGBA32I: return GL_INT;

                // Special packed integer format
                case GL_RGB10_A2UI: return GL_UNSIGNED_INT_2_10_10_10_REV;

                // Normalized unsigned formats (non-integer)
                case GL_R8: return GL_UNSIGNED_BYTE;
                case GL_RG8: return GL_UNSIGNED_BYTE;
                case GL_RGBA8: return GL_UNSIGNED_BYTE;
                case GL_SRGB8: return GL_UNSIGNED_BYTE;
                case GL_SRGB8_ALPHA8: return GL_UNSIGNED_BYTE;
                case GL_R16: return GL_UNSIGNED_SHORT;
                case GL_RG16: return GL_UNSIGNED_SHORT;
                case GL_RGBA16: return GL_UNSIGNED_SHORT;

                // Floating point formats
                case GL_R16F: return GL_FLOAT;
                case GL_RG16F: return GL_FLOAT;
                case GL_RGBA16F: return GL_FLOAT;
                case GL_R32F: return GL_FLOAT;
                case GL_RG32F: return GL_FLOAT;
                case GL_RGBA32F: return GL_FLOAT;
                // Depth and depth-stencil formats (if needed)
                // case GL_DEPTH_COMPONENT16:       return GL_UNSIGNED_SHORT;
                // case GL_DEPTH_COMPONENT24:       return GL_UNSIGNED_INT;
                // case GL_DEPTH_COMPONENT32F:      return GL_FLOAT;
                // case GL_DEPTH24_STENCIL8:        return GL_UNSIGNED_INT_24_8;
                // case GL_DEPTH32F_STENCIL8:       return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
                default:
                    std::cout <<
                            "OpenGLUtil::InternalFormatToType(GLenum internalFormat) failed Unsupported internal format\n";
                    return 0;
            }
        }
    };

    FrameBuffer::FrameBuffer(const char *name, uint32_t width, uint32_t height)
    {
        create(name, width, height);
    }

    void FrameBuffer::create(const char *name, uint32_t width, uint32_t height)
    {
        _impl = new Impl;
        glCreateFramebuffers(1, &_handle);
        this->_name = name;
        this->_width = width;
        this->_height = height;
    }

    void FrameBuffer::clean_up()
    {
        _impl->color_attachments.clear();
        glDeleteFramebuffers(1, &_handle);
        _handle = 0;
        delete _impl;
    }

    void FrameBuffer::create_attachment(const char *name, uint32_t internal_format, int32_t min_filter,
                                        int32_t mag_filter)
    {
        ColorAttachment &color_attachment = _impl->color_attachments.emplace_back();
        color_attachment.name = name;
        color_attachment.internal_format = internal_format;
        color_attachment.format = Impl::translate_internal_format_to_format(internal_format);
        color_attachment.type = Impl::translate_internal_format_to_type(internal_format);

        glCreateTextures(GL_TEXTURE_2D, 1, &color_attachment.handle);
        glTextureStorage2D(color_attachment.handle, 1, internal_format, _width, _height);
        glTextureParameteri(color_attachment.handle, GL_TEXTURE_MIN_FILTER, min_filter);
        glTextureParameteri(color_attachment.handle, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTextureParameteri(color_attachment.handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(color_attachment.handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glNamedFramebufferTexture(_handle, GL_COLOR_ATTACHMENT0 + _impl->color_attachments.size() - 1,
                                  color_attachment.handle, 0);
        std::string debugLabel = "Texture (FBO: " + std::string(_name) + " Tex: " + std::string(name) + ")";
        glObjectLabel(GL_TEXTURE, color_attachment.handle, static_cast<GLsizei>(debugLabel.length()),
                      debugLabel.c_str());
    }

    void FrameBuffer::create_depth_attachment(uint32_t internal_format, int32_t min_filter, int32_t mag_filter,
                                              int32_t wrap, glm::vec4 border_color)
    {
        _depth_attachment.internal_format = internal_format;
        glCreateTextures(GL_TEXTURE_2D, 1, &_depth_attachment.handle);
        glTextureStorage2D(_depth_attachment.handle, 1, internal_format, _width, _height);
        glTextureParameteri(_depth_attachment.handle, GL_TEXTURE_MIN_FILTER, min_filter);
        glTextureParameteri(_depth_attachment.handle, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTextureParameteri(_depth_attachment.handle, GL_TEXTURE_WRAP_S, wrap);
        glTextureParameteri(_depth_attachment.handle, GL_TEXTURE_WRAP_T, wrap);
        glTextureParameterfv(_depth_attachment.handle, GL_TEXTURE_BORDER, glm::value_ptr(border_color));
        glNamedFramebufferTexture(_handle, GL_DEPTH_ATTACHMENT, _depth_attachment.handle, 0);
        std::string debugLabel = "Depth (FBO: " + std::string(_name) + "Tex: Depth)";
        glObjectLabel(GL_TEXTURE, _depth_attachment.handle, static_cast<GLsizei>(debugLabel.length()),
                      debugLabel.c_str());
    }

    void FrameBuffer::bind_depth_attachment(const FrameBuffer &src_frame_buffer)
    {
        glNamedFramebufferTexture(_handle, GL_DEPTH_ATTACHMENT, src_frame_buffer._handle, 0);
    }

    void FrameBuffer::bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _handle);
    }

    void FrameBuffer::release()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FrameBuffer::set_empty()
    {
        bind();
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, _width);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, _height);
        release();
    }

    void FrameBuffer::set_viewport()
    {
        glViewport(0, 0, _width, _height);
    }

    void FrameBuffer::draw_buffers(const char **attachment_names, uint32_t num_attachments)
    {
        std::vector<uint32_t> attachments;
        for (int i = 0; i < num_attachments; i++)
        {
            attachments.push_back(get_color_attachment_slot_by_name(attachment_names[i]));
        }
        glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
    }

    void FrameBuffer::draw_buffer(const char *attachment_name)
    {
        for (size_t i = 0; i < _impl->color_attachments.size(); i++)
        {
            if (strcmp(attachment_name, _impl->color_attachments[i].name) == 0)
            {
                glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
                return;
            }
        }
    }

    void FrameBuffer::clear_attachment(const char *attachment_name, float_t r, float_t g, float_t b, float_t a)
    {
        for (size_t i = 0; i < _impl->color_attachments.size(); i++)
        {
            if (strcmp(attachment_name, _impl->color_attachments[i].name) == 0)
            {
                uint32_t texture = _impl->color_attachments[i].handle;
                uint32_t format = _impl->color_attachments[i].format;
                uint32_t type = _impl->color_attachments[i].type;
                float_t clear_color[4] = {r, g, b, a};
                glClearTexSubImage(texture, 0, 0, 0, 0,
                                   get_width(), get_height(), 1, format, type, clear_color);
                return;
            }
        }
    }

    void FrameBuffer::clear_attachment(const char *attachment_name, int32_t r, int32_t g, int32_t b, int32_t a)
    {
        for (size_t i = 0; i < _impl->color_attachments.size(); i++)
        {
            if (strcmp(attachment_name, _impl->color_attachments[i].name) == 0)
            {
                uint32_t texture = _impl->color_attachments[i].handle;
                uint32_t format = _impl->color_attachments[i].format;
                uint32_t type = _impl->color_attachments[i].type;
                int32_t clear_color[4] = {r, g, b, a};
                glClearTexSubImage(texture, 0, 0, 0, 0,
                                   get_width(), get_height(), 1, format, type, clear_color);
                return;
            }
        }
    }

    void FrameBuffer::clear_attachment(const char *attachment_name, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
    {
        for (size_t i = 0; i < _impl->color_attachments.size(); i++)
        {
            if (strcmp(attachment_name, _impl->color_attachments[i].name) == 0)
            {
                uint32_t texture = _impl->color_attachments[i].handle;
                uint32_t format = _impl->color_attachments[i].format;
                uint32_t type = _impl->color_attachments[i].type;
                uint32_t clear_color[4] = {r, g, b, a};
                glClearTexSubImage(texture, 0, 0, 0, 0,
                                   get_width(), get_height(), 1, format, type, clear_color);
                return;
            }
        }
    }

    void FrameBuffer::clear_depth_attachment()
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void FrameBuffer::resize(uint32_t width, uint32_t height)
    {
        _width = width;
        _height = height;

        for (size_t i = 0; i < _impl->color_attachments.size(); i++)
        {
            ColorAttachment &color_attachment = _impl->color_attachments[i];
            glDeleteTextures(1, &color_attachment.handle);
            glCreateTextures(GL_TEXTURE_2D, 1, &color_attachment.handle);
            glTextureStorage2D(color_attachment.handle, 1,
                               color_attachment.internal_format, _width, _height);
            glTextureParameteri(color_attachment.handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(color_attachment.handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(color_attachment.handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(color_attachment.handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glNamedFramebufferTexture(_handle, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i), color_attachment.handle,
                                      0);
            std::string debugLabel = "Texture (FBO: " + std::string(_name) + " Tex: " + std::string(
                                         color_attachment.name) + ")";
            glObjectLabel(GL_TEXTURE, color_attachment.handle, static_cast<GLsizei>(debugLabel.length()),
                          debugLabel.c_str());
        }

        if (_depth_attachment.handle != 0)
        {
            glDeleteTextures(1, &_depth_attachment.handle);
            glCreateTextures(GL_TEXTURE_2D, 1, &_depth_attachment.handle);
            glTextureStorage2D(_depth_attachment.handle, 1,
                               _depth_attachment.internal_format, _width, _height);
            glTextureParameteri(_depth_attachment.handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(_depth_attachment.handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(_depth_attachment.handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(_depth_attachment.handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glNamedFramebufferTexture(_handle, GL_DEPTH_ATTACHMENT, _depth_attachment.handle, 0);
            std::string debugLabel = "Texture (FBO: " + std::string(_name) + " Tex: Depth)";
            glObjectLabel(GL_TEXTURE, _depth_attachment.handle, static_cast<GLsizei>(debugLabel.length()),
                          debugLabel.c_str());
        }
    }

    uint32_t FrameBuffer::get_width() const
    {
        return _width;
    }

    uint32_t FrameBuffer::get_height() const
    {
        return _height;
    }

    uint32_t FrameBuffer::get_color_attachment_handle_by_name(const char *name) const
    {
        for (size_t i = 0; i < _impl->color_attachments.size(); i++)
        {
            if (strcmp(name, _impl->color_attachments[i].name) == 0)
            {
                return _impl->color_attachments[i].handle;
            }
        }
        LOG_ERROR("COULDNT FIND ATTACHMENT WITH NAME %s", name);
        return GL_NONE;
    }

    uint32_t FrameBuffer::get_handle() const
    {
        return _handle;
    }


    uint32_t FrameBuffer::get_depth_attachment_handle() const
    {
        return _depth_attachment.handle;
    }

    uint32_t FrameBuffer::get_color_attachment_slot_by_name(const char *name) const
    {
        for (size_t i = 0; i < _impl->color_attachments.size(); i++)
        {
            if (strcmp(name, _impl->color_attachments[i].name) == 0)
            {
                return GL_COLOR_ATTACHMENT0 + i;
            }
        }
        LOG_ERROR("COULDNT FIND ATTACHMENT SLOT with name %s", name);
        return GL_INVALID_VALUE;
    }

    void FrameBuffer::blit_to_default_frame_buffer(const char *srcName, int32_t dstX0, int32_t dstY0, int32_t dstX1,
                                                   int32_t dstY1, uint32_t mask, uint32_t filter)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, get_handle());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glReadBuffer(get_color_attachment_slot_by_name(srcName));
        glDrawBuffer(GL_BACK);
        glBlitFramebuffer(0, 0, _width, _height, dstX0, dstY0, dstX1, dstY1, mask, filter);
    }

    bool FrameBuffer::is_valid() const
    {
        return _handle != 0;
    }
}
