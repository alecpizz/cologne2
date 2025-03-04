#pragma once
namespace goon
{
    struct VertexAttribute
    {
        uint32_t handle;
        VertexAttribute()
        {
            glGenVertexArrays(1, &handle);
            glBindVertexArray(handle);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                sizeof(Vertex), static_cast<void *>(0));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, uv)));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, tangent)));
            glBindVertexArray(0);
        }

        void use() const
        {
            glBindVertexArray(handle);
        }

        void release() const
        {
            glBindVertexArray(0);
        }

        ~VertexAttribute()
        {
            glDeleteVertexArrays(1, &handle);
        }
    };
}
