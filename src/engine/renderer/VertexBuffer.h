#pragma once
#include "gpch.h"
#include "Vertex.h"
namespace goon
{
    struct VertexBuffer
    {
        uint32_t handle = 0;

        VertexBuffer(const Vertex* vertices, const uint32_t size)
        {
            glGenBuffers(1, &handle);
            glBindBuffer(GL_ARRAY_BUFFER, handle);
            glBufferData(GL_ARRAY_BUFFER, size * sizeof(Vertex), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        void use() const
        {
            glBindBuffer(GL_ARRAY_BUFFER, handle);
        }

        void release() const
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        ~VertexBuffer()
        {
            glDeleteBuffers(1, &handle);
        }
    };
}
