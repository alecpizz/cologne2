#pragma once

namespace cologne
{
    class SSBO
    {
    public:
        SSBO() = default;
        SSBO(size_t size, uint32_t flags);
        uint32_t get_handle() const;
        void pre_allocate(size_t size);
        void update(size_t size, const void* data);
        void bind(int32_t index) const;
        void cleanup();
        void copy_from(const void* host, size_t size);
    private:
        uint32_t _flags;
        uint32_t _handle;
        size_t _buffer_size;
    };
}
