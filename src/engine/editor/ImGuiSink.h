//
// Created by alecpizz on 8/30/25.
//
#pragma once
#include <shared_mutex>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>


namespace cologne
{
    class ImGuiSink : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        static const char* const logger_name;
    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override;

        void flush_() override;
    };
}
