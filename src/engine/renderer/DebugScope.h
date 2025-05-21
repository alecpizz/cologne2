#pragma once

namespace cologne
{
    class DebugScope
    {
        inline static uint32_t global_scope_depth;
        const uint32_t scope_depth;

    public:
        explicit DebugScope(const std::string& scope_name) : scope_depth(global_scope_depth++)
        {
            glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, scope_depth, scope_name.size(), scope_name.data());
        }
        ~DebugScope()
        {
            glPopDebugGroup();
            global_scope_depth--;
        }
    };
}
