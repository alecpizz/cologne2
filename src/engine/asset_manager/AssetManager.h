#pragma once
#include <engine/renderer/types/Model.h>
#include <engine/renderer/types/SkinnedModel.h>
#include <engine/animation/Animation.h>

namespace cologne::AssetManager
{
    void init();
    void find_file_paths();
    void load_model(const std::string& path);
    void load_skinned_model(const std::string& path);
    void load_animations(const std::string& path);
    void print_animations();
    void print_models();
    void print_skinned_models();
    void print_all();
    std::vector<Animation>& get_animations();
    std::vector<Model>& get_models();
    std::vector<SkinnedModel>& get_skinned_models();

    Model* get_model_by_name(const std::string& name);
    Model* get_model_by_index(size_t idx);
    size_t get_model_index_by_name(const std::string& name);

    SkinnedModel* get_skinned_model_by_name(const std::string& name);
    SkinnedModel* get_skinned_model_by_index(size_t idx);
    size_t get_skinned_model_index_by_name(const std::string& name);

    Animation* get_animation_by_name(const std::string& name);
    Animation* get_animation_by_index(size_t idx);
    size_t get_animation_index_by_name(const std::string& name);
}
