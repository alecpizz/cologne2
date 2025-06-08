//
// Created by alecpizz on 3/1/2025.
//

#include "SSBO.h"

namespace cologne
{
    SSBO::SSBO(size_t size, uint32_t flags)
    {
        _flags = flags;
        pre_allocate(size);
    }

    uint32_t SSBO::get_handle() const
    {
        return _handle;
    }

    void SSBO::pre_allocate(size_t size)
    {
        cleanup();
        glCreateBuffers(1, &_handle);
        glNamedBufferStorage(_handle, static_cast<GLsizeiptr>(size), nullptr, _flags);
        _buffer_size = size;
    }

    void SSBO::update(size_t size, const void *data)
    {
        if (size == 0 || data == nullptr)
        {
            return;
        }
        if (_handle == 0 || _buffer_size < size)
        {
            cleanup();
            glCreateBuffers(1, &_handle);
            glNamedBufferStorage(_handle, static_cast<GLsizeiptr>(size), nullptr, _flags);
            _buffer_size = size;
        }
        glNamedBufferSubData(_handle, 0, static_cast<GLsizeiptr>(size), data);
    }

    void SSBO::bind(int32_t index) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, _handle);
    }

    void SSBO::cleanup()
    {
        if (_handle != 0)
        {
            glDeleteBuffers(1, &_handle);
            _handle = 0;
            _buffer_size = 0;
        }
    }

    void SSBO::copy_from(const void *host, size_t size)
    {
        if (!host || size == 0 || _handle == 0)
        {
            return;
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _handle);
        void* basePtr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        if (!basePtr)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            throw std::runtime_error("glMapBuffer range failed in copyFrom");
        }

        std::memcpy(basePtr, host, size);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}
