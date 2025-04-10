#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

#define FORMAT_LOG_ENTRY( msg, formatted_msg ) \
char formatted_msg[ 4096 ] ; \
va_list ap ; \
va_start( ap, msg ) ; \
vsnprintf( formatted_msg, sizeof( formatted_msg ), msg, ap ) ; \
va_end( ap ) ;


namespace cologne
{
    struct Log::Impl
    {
        std::shared_ptr<spdlog::logger> logger;

        void init()
        {
            spdlog::set_pattern("%^[%T] %n: %v%$");
            logger = spdlog::stdout_color_mt("cologne");
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
        FORMAT_LOG_ENTRY(msg, formatted_msg);
        _impl->logger->log(spdlog::level::info, formatted_msg);
    }

    void Log::log_error(const char *msg, ...)
    {
        FORMAT_LOG_ENTRY(msg, formatted_msg);
        _impl->logger->log(spdlog::level::err, formatted_msg);
    }

    void Log::log_debug(const char *msg, ...)
    {
        FORMAT_LOG_ENTRY(msg, formatted_msg);
        _impl->logger->log(spdlog::level::debug, formatted_msg);
    }

    void Log::log_warning(const char *msg, ...)
    {
        FORMAT_LOG_ENTRY(msg, formatted_msg);
        _impl->logger->log(spdlog::level::warn, formatted_msg);
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
