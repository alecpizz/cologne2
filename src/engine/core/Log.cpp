#include <spdlog/spdlog.h>
#include <engine/editor/Editor.h>
#include <engine/editor/ImGuiSink.h>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

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
        std::vector<spdlog::sink_ptr> sinks;
        std::shared_ptr<spdlog::async_logger> logger;

        void init()
        {
            spdlog::set_pattern("%^[%T] %n: %v%$");
            spdlog::init_thread_pool(8192, 1);
            sinks.push_back( std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            sinks.push_back( std::make_shared<spdlog::sinks::basic_file_sink_mt>(RESOURCES_PATH "logs/cologne.log"));
            sinks.push_back(std::make_shared<ImGuiSink>());
            logger = std::make_shared<spdlog::async_logger>("cologne", std::begin(sinks), std::end(sinks), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
            spdlog::register_logger(logger);
            logger->set_level(spdlog::level::trace);
        }
        void add_sink(spdlog::sink_ptr sink)
        {
            spdlog::get("cologne")->sinks().push_back(sink);
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
