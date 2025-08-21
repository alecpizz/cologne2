//
// Created by alecpizz on 6/21/25.
//

#include "FileWatcher.h"
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>

namespace cologne
{

    FileWatcher::FileWatcher(const std::string &path, const std::function<void(std::filesystem::path, FileStatus)> &action)
    {
        _path = path;
        _action = action;
        for (auto& file : std::filesystem::recursive_directory_iterator(path))
        {
            _paths[file.path()] = std::filesystem::last_write_time(file);
        }
        _thread = std::thread(&FileWatcher::thread_task, this);
    }

    FileWatcher::~FileWatcher()
    {
        stop();
    }

    void FileWatcher::stop()
    {
        _done = true;
        _thread.join();
        LOG_INFO("File Watcher Stopped");
    }

    void FileWatcher::thread_task()
    {
        LOG_INFO("File Watcher Started");
        using namespace std::chrono_literals;
        while (!_done)
        {
            check_files();
            std::this_thread::sleep_for(1000ms);
        }
    }


    void FileWatcher::check_files()
    {
        for (auto& file : std::filesystem::recursive_directory_iterator(_path))
        {
            auto current_file_last_write_time = std::filesystem::last_write_time(file);
            if (_paths.contains(file.path()))
            {
                if (_paths[file.path()] != current_file_last_write_time)
                {
                    _paths[file.path()] = current_file_last_write_time;
                    if (_action != nullptr)
                    {
                        _action(file.path(), FileStatus::MODIFIED);
                    }
                }
            }
            else
            {
                _paths[file.path()] = current_file_last_write_time;
                if (_action)
                {
                    _action(file.path(), FileStatus::CREATED);
                }
            }
        }

        //why does this crash the program??
        // for (auto it = _paths.begin(); it != _paths.end();)
        // {
        //     if (!exists(it->first))
        //     {
        //         if (_action)
        //         {
        //             _action(it->first.string(), FileStatus::ERASED);
        //         }
        //         _paths.erase(it);
        //     }
        //     else
        //     {
        //         ++it;
        //     }
        // }
    }

}
