#pragma once

namespace cologne
{
    struct ColorAttachment
    {
        const char *name = "undefined";
        uint32_t handle = 0;
        uint32_t internal_format = 0;
        uint32_t format = 0;
        uint32_t type = 0;
    };

    struct DepthAttachment
    {
        uint32_t handle = 0;
        uint32_t internal_format = 0;
    };

    class FrameBuffer
    {
    public:
        FrameBuffer() = default;

        FrameBuffer(const char *name, uint32_t width, uint32_t height);

        void create(const char *name, uint32_t width, uint32_t height);

        void clean_up();

        void create_attachment(const char *name, uint32_t internal_format, int32_t min_filter, int32_t mag_filter);

        void create_depth_attachment(uint32_t internal_format, int32_t min_filter, int32_t mag_filter, int32_t wrap,
                                     glm::vec4 border_color = glm::vec4(1.0f));

        void bind_depth_attachment(const FrameBuffer &src_frame_buffer);

        void bind();

        void release();

        void set_empty();

        void set_viewport();

        void draw_buffers(const char **attachment_names, uint32_t num_attachments);

        void draw_buffer(const char *attachment_name);

        void clear_attachment(const char *attachment_name, float_t r, float_t g = 0.0f, float_t b = 0.0f,
                              float_t a = 0.0f);

        void clear_attachment(const char *attachment_name, int32_t r, int32_t g = 0, int32_t b = 0, int32_t a = 0);

        void clear_attachment(const char *attachment_name, uint32_t r, uint32_t g = 0, uint32_t b = 0, uint32_t a = 0);

        void clear_depth_attachment();

        void resize(uint32_t width, uint32_t height);

        uint32_t get_width() const;

        uint32_t get_height() const;

        uint32_t get_color_attachment_handle_by_name(const char *name) const;

        uint32_t get_handle() const;

        uint32_t get_depth_attachment_handle() const;

        uint32_t get_color_attachment_slot_by_name(const char *name) const;

        void blit_to_default_frame_buffer(const char *srcName, int32_t dstX0, int32_t dstY0, int32_t dstX1,
                                          int32_t dstY1, uint32_t mask, uint32_t filter);
        bool is_valid() const;

    private:
        const char *_name = "undefined";
        uint32_t _handle = 0;
        uint32_t _width = 0;
        uint32_t _height = 0;
        DepthAttachment _depth_attachment;
        std::vector<ColorAttachment> _color_attachments;
        struct Impl;
        Impl *_impl = nullptr;
    };
}
