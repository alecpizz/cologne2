#pragma once
#include "Types.h"
#include "../gpch.h"

namespace cologne::FileUtil
{
    ModelData import_model(const std::string& path);
    SkinnedModelData import_skinned_model(const std::string& path);
    std::string get_file_name(const std::string& path);
}
