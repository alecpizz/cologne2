#include "Log.h"
#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace goon
{
    struct Log::Impl
    {
        std::shared_ptr<spdlog::logger> logger;

        void init()
        {
            spdlog::set_pattern("%^[%T] %n: %v%$");
            logger = spdlog::stdout_color_mt("goon");
            logger->set_level(spdlog::level::trace);
        }
    };

    Log & Log::get()
    {
        if (s_instance == nullptr)
        {
            s_instance = new Log();
        }
        return *s_instance;
    }

    void Log::log_info(const char *msg, ...)
    {
        _impl->logger->log(spdlog::level::info, msg);
    }

    void Log::log_error(const char *msg, ...)
    {
        _impl->logger->log(spdlog::level::err, msg);
    }

    void Log::log_debug(const char *msg, ...)
    {
        _impl->logger->log(spdlog::level::debug, msg);
    }

    void Log::log_warning(const char *msg, ...)
    {
        _impl->logger->log(spdlog::level::warn, msg);
    }

    Log::Log()
    {
        _impl = new Impl;
        _impl->init();
    }

    Log::~Log()
    {
        spdlog::shutdown();
        delete _impl;
    }
}
