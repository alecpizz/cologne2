//
// Created by alecpizz on 8/30/25.
//

#include "ImGuiSink.h"
#include "Editor.h"

namespace cologne
{
    const char *const ImGuiSink::logger_name = "ui_log";

    ImVec4 level_to_color(spdlog::level::level_enum level) {
        switch (level) {
            case spdlog::level::trace:    return ImVec4(0.75f, 0.75f, 0.75f, 1.0f); // Gray
            case spdlog::level::debug:    return ImVec4(0.5f, 1.0f, 0.5f, 1.0f);   // Light Green
            case spdlog::level::info:     return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // White
            case spdlog::level::warn:     return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);   // Yellow
            case spdlog::level::err:      return ImVec4(1.0f, 0.2f, 0.2f, 1.0f);   // Red
            case spdlog::level::critical: return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);   // Bright Red
            default:                      return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    void ImGuiSink::sink_it_(const spdlog::details::log_msg &msg)
    {
        spdlog::memory_buf_t formatted;
        this->formatter_->format(msg, formatted);
        // std::lock_guard<std::mutex> lock(this->mutex_);
        LogRecord record = {};
        record.message = fmt::to_string(formatted);
        record.color = level_to_color(msg.level);
        Editor::submit_log_record(record);
    }

    void ImGuiSink::flush_()
    {
    }
}
