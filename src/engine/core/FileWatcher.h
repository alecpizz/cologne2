//
// Created by alecpizz on 6/21/25.
//
#pragma once
#include <thread>
#include <filesystem>

namespace cologne
{
    enum class FileStatus
    {
        CREATED,
        MODIFIED,
        ERASED
    };
    class FileWatcher
    {
    public:
        FileWatcher(const std::string& path, const std::function<void(std::filesystem::path, FileStatus)> &action);
        ~FileWatcher();
        void stop();
    private:
        void thread_task();
        void check_files();
        std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> _paths;
        std::thread _thread;
        std::string _path = std::string();
        std::function<void(std::filesystem::path, FileStatus)> _action;
        bool _done = false;
    };
}
