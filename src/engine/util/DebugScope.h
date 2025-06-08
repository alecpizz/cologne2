#pragma once

namespace cologne
{
    class DebugScope
    {
        std::string _name;
        std::chrono::time_point<std::chrono::steady_clock> time;

    public:
        DebugScope(const std::string &name)
        {
            _name = name;
            time = std::chrono::high_resolution_clock::now();
        }

        ~DebugScope()
        {
            auto time2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> ms = time2 - time;
            LOG_INFO("\n\n%s %f ms", _name.c_str(), ms.count());
        }
    };
}
