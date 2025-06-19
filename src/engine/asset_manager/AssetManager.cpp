//
// Created by alecpizz on 6/2/25.
//

#include "AssetManager.h"
#include <filesystem>
#include <engine/util/DebugScope.h>
#include <engine/util/FileUtil.h>

namespace cologne::AssetManager
{
    std::vector<Model> models;
    std::vector<SkinnedModel> skinned_models;
    std::vector<Animation> animations;
    std::unordered_map<std::string, size_t> model_index_map;
    std::unordered_map<std::string, size_t> skinned_model_index_map;
    std::unordered_map<std::string, size_t> animation_index_map;
    bool is_loading = true;

    void init()
    {
        DebugScope scope(__PRETTY_FUNCTION__);
        find_file_paths();
        for (size_t i = 0; i < models.size(); i++)
        {
            model_index_map[models[i].get_name()] = i;
        }

        for (size_t i = 0; i < skinned_models.size(); i++)
        {
            skinned_model_index_map[skinned_models[i].get_name()] = i;
        }

        for (size_t i = 0; i < animations.size(); i++)
        {
            animation_index_map[animations[i].get_name()] = i;
        }
        print_all();
    }

    void find_file_paths()
    {
        DebugScope scope(__PRETTY_FUNCTION__);

        std::vector<FileUtil::FileInfo> model_paths = FileUtil::iterate_directory(RESOURCES_PATH "models");
        std::vector<ModelData> model_datas;
        model_datas.resize(model_paths.size());
        std::transform(std::execution::par_unseq, std::begin(model_paths),
                       std::end(model_paths), std::begin(model_datas),
                       [](const FileUtil::FileInfo &file)
                       {
                           const ModelData data = FileUtil::import_model(file.path);
                           return data;
                       });

        //skinned models --> assuming animations are in models for now
        std::vector<FileUtil::FileInfo> skinned_paths = FileUtil::iterate_directory(RESOURCES_PATH "skinned_models");
        std::vector<SkinnedModelData> skinned_model_datas;
        skinned_model_datas.resize(skinned_paths.size());
        std::transform(std::execution::par_unseq, std::begin(skinned_paths), std::end(skinned_paths),
                       std::begin(skinned_model_datas), [](const FileUtil::FileInfo &file)
                       {
                           const SkinnedModelData data = FileUtil::import_skinned_model(file.path);
                           return data;
                       });
        scope = DebugScope("load models");
        for (auto &model_data: model_datas)
        {
            for (auto &material: model_data.materials)
            {
                material.load_all();
            }
            models.emplace_back(model_data);
        }
        for (auto &skinned_model_data: skinned_model_datas)
        {
            for (auto &material: skinned_model_data.materials)
            {
                material.load_all();
            }

            animations.insert(animations.end(),
                              skinned_model_data.animations.begin(), skinned_model_data.animations.end());
            skinned_models.emplace_back(skinned_model_data);
        }
        // for (auto &info: FileUtil::iterate_directory(RESOURCES_PATH "skinned_models"))
        // {
        //     auto model_data = FileUtil::import_skinned_model(info.path);
        //     if (!model_data.meshes.empty())
        //     {
        //         animations.insert(animations.end(),
        //                           model_data.animations.begin(), model_data.animations.end());
        //         skinned_models.emplace_back(model_data);
        //     }
        // }
    }

    void load_model(const std::string &path)
    {
        auto data = FileUtil::import_model(path);
        if (!data.meshes.empty())
        {
            models.emplace_back(data);
        }
    }

    void load_skinned_model(const std::string &path)
    {
        auto data = FileUtil::import_skinned_model(path);
        if (!data.meshes.empty())
        {
            skinned_models.emplace_back(data);
        }
    }

    std::vector<Model> &get_models()
    {
        return models;
    }

    std::vector<SkinnedModel> &get_skinned_models()
    {
        return skinned_models;
    }

    void print_models()
    {
        for (const auto &model: models)
        {
            LOG_INFO("MODEL %s", model.get_name());
        }
    }

    void print_skinned_models()
    {
        for (const auto &model: skinned_models)
        {
            LOG_INFO("SKINNED MODEL %s", model.get_name().c_str());
        }
    }

    void print_all()
    {
        print_models();
        print_skinned_models();
        print_animations();
    }

    void print_animations()
    {
        for (auto &anim: animations)
        {
            LOG_INFO("ANIMATION %s", anim.get_name().c_str());
        }
    }

    std::vector<Animation> &get_animations()
    {
        return animations;
    }


    Model *get_model_by_name(const std::string &name)
    {
        return &models.at(get_model_index_by_name(name));
    }

    Model *get_model_by_index(size_t idx)
    {
        return &models.at(idx);
    }

    size_t get_model_index_by_name(const std::string &name)
    {
        return model_index_map[name];
    }

    SkinnedModel *get_skinned_model_by_name(const std::string &name)
    {
        return &skinned_models.at(get_skinned_model_index_by_name(name));
    }

    SkinnedModel *get_skinned_model_by_index(size_t idx)
    {
        return &skinned_models.at(idx);
    }

    size_t get_skinned_model_index_by_name(const std::string &name)
    {
        return skinned_model_index_map[name];
    }

    Animation *get_animation_by_name(const std::string &name)
    {
        return &animations.at(animation_index_map[name]);
    }

    Animation *get_animation_by_index(size_t idx)
    {
        return &animations.at(idx);
    }

    size_t get_animation_index_by_name(const std::string &name)
    {
        return animation_index_map[name];
    }
}
