#pragma once

namespace cologne
{
    class DebugScope
    {
        inline static uint32_t global_scope_depth;
        const uint32_t scope_depth;
        std::string _name;
        std::chrono::time_point<std::chrono::system_clock> time;
    public:
        explicit DebugScope(const std::string& name) : scope_depth(global_scope_depth--)
        {
            _name = name;
            time = std::chrono::high_resolution_clock::now();
        }

        ~DebugScope()
        {
            auto time2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> ms = time2 - time;
            LOG_INFO("\n\n%s %f ms", _name.c_str(), ms.count());
            global_scope_depth--;
        }
    };
}
