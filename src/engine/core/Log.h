#pragma once

#define LOG_INFO( ... )   Log::get().log_info( __VA_ARGS__ )
#define LOG_ERROR( ... )  Log::get().log_error( __VA_ARGS__ )
#define LOG_DEBUG( ... )     Log::get().log_debug( __VA_ARGS__ )
#define LOG_WARN( ... )    Log::get().log_warning( __VA_ARGS__ )

namespace cologne
{
    class Log
    {
    public:
        static Log &get();

        ~Log();

        Log(Log &&) = delete;

        Log(const Log &) = delete;

        Log &operator=(Log &&) = delete;

        Log &operator=(const Log &) = delete;

        void log_info(const char *msg, ...);

        void log_error(const char *msg, ...);

        void log_debug(const char *msg, ...);

        void log_warning(const char *msg, ...);

    private:
        Log();

        inline static Log *s_instance = nullptr;
        struct Impl;
        Impl *_impl;
    };
}
