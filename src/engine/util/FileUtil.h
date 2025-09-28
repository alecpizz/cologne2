#pragma once
#include <engine/Types.h>
#include <filesystem>

namespace cologne::FileUtil
{
    struct FileInfo
    {
        std::string path;
        std::string name;
        std::string ext;
        std::string dir;
    };

    ModelData import_model(const std::string &path);

    SkinnedModelData import_skinned_model(const std::string &path);

    std::string get_file_name(const std::string &path);

    std::string get_full_path(const std::filesystem::directory_entry &entry);

    std::string get_file_extension(const std::filesystem::directory_entry &entry);

    std::string get_file_name_without_extension(const std::filesystem::directory_entry &entry);

    std::vector<FileInfo> iterate_directory(const std::string &directory,
                                            const std::vector<std::string> &extensions = {});
    //TODO texture data, texture managemetn shit
    Texture import_exr(const std::string& path);

    bool create_directory_recursive(const std::string& directory);
    bool file_exists(const std::string& path);
}
