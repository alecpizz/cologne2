#pragma once

namespace goon
{
    struct ElementBuffer
    {
        uint32_t handle = 0;

        ElementBuffer() = default;

        ElementBuffer(const uint32_t* indices, const uint32_t numIndices)
        {
            glGenBuffers(1, &handle);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32_t), &indices, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        void use() const
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        }

        void release() const
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        ~ElementBuffer()
        {
            glDeleteBuffers(1, &handle);
        }
    };
}
