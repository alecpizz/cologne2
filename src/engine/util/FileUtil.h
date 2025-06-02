#pragma once
#include <engine/Types.h>
#include <engine/animation/Animation.h>

namespace cologne::FileUtil
{
    ModelData import_model(const std::string& path);
    std::vector<Animation> import_animations(const std::string& path, SkinnedModel& model);
    SkinnedModelData import_skinned_model(const std::string& path);
    std::string get_file_name(const std::string& path);
}
