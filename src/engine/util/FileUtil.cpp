//
// Created by alecp on 5/10/2025.
//

#include "FileUtil.h"

#include "assimp/Importer.hpp"
#include <filesystem>


namespace cologne::FileUtil
{

    std::string get_file_name(const std::string &path)
    {
        size_t pos = path.find_last_of("/\\");
        std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
        pos = filename.find_last_of('.');
        return (pos != std::string::npos) ? filename.substr(0, pos) : filename;
    }

    std::string get_full_path(const std::filesystem::directory_entry &entry)
    {
        return entry.path().string();
    }

    std::string get_file_extension(const std::filesystem::directory_entry &entry)
    {
        return entry.path().extension().string().substr(1);
    }

    std::string get_file_name_without_extension(const std::filesystem::directory_entry &entry)
    {
        return entry.path().stem().string();
    }

    std::vector<FileInfo> iterate_directory(const std::string &directory, const std::vector<std::string> &extensions)
    {
        std::vector<FileInfo> result;
        if (!std::filesystem::exists(directory))
        {
            return result;
        }

        auto iter = std::filesystem::directory_iterator(directory);
        for (const auto &entry: iter)
        {
            if (!std::filesystem::is_regular_file(entry)) continue;

            FileInfo info = {
                get_full_path(entry), get_file_name_without_extension(entry), get_file_extension(entry), directory
            };

            if (extensions.empty() || std::find(extensions.begin(), extensions.end(), info.ext) != extensions.end())
            {
                result.push_back(info);
            }
        }
        return result;
    }

    bool create_directory_recursive(const std::string &directory)
    {
        std::filesystem::path path(directory);
        if (!std::filesystem::create_directories(path.parent_path()))
        {
            if (std::filesystem::exists(path.parent_path()))
            {
                return true;
            }
            return false;
        }
        return true;
    }

    bool file_exists(const std::string &path)
    {
        return std::filesystem::exists(path);
    }
}
