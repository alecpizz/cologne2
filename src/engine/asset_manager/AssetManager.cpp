//
// Created by alecpizz on 6/2/25.
//

#include "AssetManager.h"
#include <filesystem>
#include <engine/util/DebugScope.h>
#include <engine/util/FileUtil.h>
#include <algorithm>
#include <string>
#include <vector>
#include <engine/core/Engine.h>
#include <engine/renderer/Renderer.h>
#include <chrono>

namespace cologne::AssetManager
{
    std::vector<Model> models;
    std::vector<SkinnedModel> skinned_models;
    std::vector<AnimationClip> animations;
    std::vector<Mesh> meshes;
    std::vector<SkinnedMesh> skinned_meshes;
    std::vector<Material> materials;
    std::unordered_map<std::string, int32_t> model_index_map;
    std::unordered_map<std::string, int32_t> skinned_model_index_map;
    std::unordered_map<std::string, int32_t> animation_index_map;
    std::unordered_map<std::string, int32_t> mesh_index_map;
    std::unordered_map<std::string, int32_t> skinned_mesh_index_map;
    bool is_loading = true;
    int32_t material_offset = 0;
    std::vector<Vertex> vertices;
    std::vector<WeightedVertex> weighted_vertices;
    std::vector<uint32_t> indices;
    std::vector<uint32_t> weighted_indices;
    int32_t mesh_offset = 0;
    int32_t skinned_mesh_offset = 0;
    int32_t first_index = 0;
    int32_t base_vertex = 0;
    int32_t first_weighted_index = 0;
    int32_t base_weighted_vertex = 0;

    void init()
    {
        DebugScope scope(__PRETTY_FUNCTION__);
        find_file_paths();
        for (int32_t i = 0; i < models.size(); i++)
        {
            model_index_map[models[i].get_name()] = i;
        }

        for (int32_t i = 0; i < skinned_models.size(); i++)
        {
            skinned_model_index_map[skinned_models[i].get_name()] = i;
        }

        for (int32_t i = 0; i < animations.size(); i++)
        {
            animation_index_map[animations[i].get_name()] = i;
        }

        for (int32_t i = 0; i < meshes.size(); i++)
        {
            mesh_index_map[meshes[i].get_name()] = i;
        }

        for (int32_t i = 0; i < skinned_meshes.size(); i++)
        {
            skinned_mesh_index_map[skinned_meshes[i].get_name()] = i;
        }
        print_all();
    }

    void find_file_paths()
    {
        DebugScope scope(__PRETTY_FUNCTION__);

        std::vector<FileUtil::FileInfo> model_paths = FileUtil::iterate_directory(RESOURCES_PATH "models");
        std::vector<ModelData> model_datas;
        for (auto& model_path : model_paths)
        {
            if (!std::filesystem::exists(RESOURCES_PATH "cache/models/" + model_path.name + ".cmdl"))
            {
                LOG_INFO("No cache for file %s found!", model_path.name.c_str());
                const ModelData data = FileUtil::import_model(model_path.path);
                auto time = std::filesystem::last_write_time(model_path.path);
                auto sys_time = std::chrono::clock_cast<std::chrono::system_clock>(time);
                export_model(data, std::chrono::duration_cast<std::chrono::seconds>(sys_time.time_since_epoch()).count());
            }
        }

        std::vector<FileUtil::FileInfo> cache_model_paths = FileUtil::iterate_directory(RESOURCES_PATH "cache/models");
        model_datas.resize(cache_model_paths.size());
        std::transform(std::execution::par_unseq, std::begin(cache_model_paths),
                       std::end(cache_model_paths), std::begin(model_datas),
                       [](const FileUtil::FileInfo &file)
                       {
                           const ModelData data = import_model(file.path.c_str());
                           return data;
                       });

        //skinned models --> assuming animations are in models for now
        std::vector<FileUtil::FileInfo> skinned_paths = FileUtil::iterate_directory(RESOURCES_PATH "skinned_models");
        std::vector<SkinnedModelData> skinned_model_datas;
        for (auto& model_path : skinned_paths)
        {
            if (!std::filesystem::exists(RESOURCES_PATH "cache/skinned_models/" + model_path.name + ".cskmdl"))
            {
                LOG_INFO("No cache for file %s found!", model_path.name.c_str());
                const SkinnedModelData data = FileUtil::import_skinned_model(model_path.path);
                auto time = std::filesystem::last_write_time(model_path.path);
                auto sys_time = std::chrono::clock_cast<std::chrono::system_clock>(time);
                export_skinned_model(data, std::chrono::duration_cast<std::chrono::seconds>(sys_time.time_since_epoch()).count());
                LOG_INFO("Exported skinned model %s", model_path.name.c_str());
            }
        }

        std::vector<FileUtil::FileInfo> cache_skinned_paths = FileUtil::iterate_directory(RESOURCES_PATH "cache/skinned_models");
        skinned_model_datas.resize(cache_skinned_paths.size());
        std::transform(std::execution::par_unseq, std::begin(cache_skinned_paths), std::end(cache_skinned_paths),
                       std::begin(skinned_model_datas), [](const FileUtil::FileInfo &file)
                       {
                           const SkinnedModelData data = import_skinned_model(file.path.c_str());
                           return data;
                       });
        scope = DebugScope("load models");
        //DO MESHES BEFORE MODELS

        for (auto &model_data: model_datas)
        {
            int32_t startingIdx = material_offset; //our current index in the big ol material buffer
            for (Material &mat: model_data.materials)
            {
                materials.emplace_back(mat);
                material_offset++; //add however many materials are in the model
            }

            for (MeshData &mesh: model_data.meshes)
            {
                mesh.material_index = startingIdx + mesh.material_index;
                //the material indices of each model are just offset from the starting pt
            }

            std::vector<int32_t> mesh_indices;
            for (auto &mesh: model_data.meshes)
            {
                mesh.base_vertex = base_vertex;
                mesh.first_index = first_index;
                vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
                indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
                base_vertex += mesh.vertices.size();
                first_index += mesh.indices.size();
                meshes.emplace_back(mesh); //create a mesh
                mesh_indices.emplace_back(mesh_offset); //add its index
                mesh_offset++; //increment the total amount
            }
            models.emplace_back(mesh_indices, model_data.name, model_data.aabb_min, model_data.aabb_max);
            //add our model
        }

        Engine::get_renderer()->upload_vertex_data(vertices, indices);

        for (auto &skinned_model_data: skinned_model_datas)
        {
            int32_t starting_idx = material_offset;
            for (Material &mat: skinned_model_data.materials)
            {
                materials.emplace_back(mat);
                material_offset++;
            }

            for (SkinnedMeshData &mesh: skinned_model_data.meshes)
            {
                mesh.material_index = starting_idx + mesh.material_index;
            }

            std::vector<int32_t> skinned_mesh_indices;
            for (auto &skinned_mesh: skinned_model_data.meshes)
            {
                skinned_mesh.base_vertex = base_weighted_vertex;
                skinned_mesh.first_index = first_weighted_index;
                weighted_vertices.insert(weighted_vertices.end(), skinned_mesh.vertices.begin(),
                                         skinned_mesh.vertices.end());
                weighted_indices.insert(weighted_indices.end(), skinned_mesh.indices.begin(),
                                        skinned_mesh.indices.end());
                base_weighted_vertex += skinned_mesh.vertices.size();
                first_weighted_index += skinned_mesh.indices.size();
                skinned_meshes.emplace_back(skinned_mesh);
                skinned_mesh_indices.emplace_back(skinned_mesh_offset);
                skinned_mesh_offset++;
            }
            skinned_models.emplace_back(skinned_mesh_indices, skinned_model_data.name, skinned_model_data.aabb_min,
                                        skinned_model_data.aabb_max, skinned_model_data.skeleton);
            animations.insert(animations.end(),
                              skinned_model_data.animations.begin(), skinned_model_data.animations.end());
        }

        Engine::get_renderer()->upload_weighted_vertex_data(weighted_vertices, weighted_indices);

        //slow texture upload step.
        for (auto &m: materials)
        {
            m.load_all();
        }
    }

    void load_model(const std::string &path)
    {
        std::vector<Mesh> new_meshes;
        size_t mesh_size = meshes.size() - 1;
        size_t model_size = models.size() - 1;
        //TODO: don't block main thread

        // auto a1 = std::async(std::launch::async, FileUtil::import_model, path);
        // a1.wait();
        // auto data = a1.get();
        auto data = FileUtil::import_model(path);
        int32_t starting_idx = material_offset;
        for (Material &mat: data.materials)
        {
            materials.emplace_back(mat);
            material_offset++; //add however many materials are in the model
        }

        for (auto &mesh: data.meshes)
        {
            mesh.material_index = starting_idx + mesh.material_index;
        }

        std::vector<int32_t> mesh_indices;
        for (auto &mesh: data.meshes)
        {
            mesh.first_index = first_index;
            mesh.base_vertex = base_vertex;
            vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
            indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
            meshes.emplace_back(mesh); //create a mesh
            base_vertex += mesh.vertices.size();
            first_index += mesh.indices.size();
            mesh_indices.emplace_back(mesh_offset); //add its index
            mesh_offset++; //increment the total amount
        }

        models.emplace_back(mesh_indices, data.name, data.aabb_min, data.aabb_max);
        Engine::get_renderer()->upload_vertex_data(vertices, indices);
        for (size_t i = starting_idx; i < materials.size(); i++)
        {
            materials[i].load_all();
        }

        model_index_map[models[model_size + 1].get_name()] = model_size + 1;
        for (size_t i = 0; i < data.meshes.size(); i++)
        {
            mesh_index_map[meshes[mesh_size + i].get_name()] = mesh_size + i;
        }
        mesh_index_map[meshes[mesh_size + data.meshes.size()].get_name()] = mesh_size + data.meshes.size();
    }

    void load_skinned_model(const std::string &path)
    {
        LOG_ERROR("implement me");
        // auto data = FileUtil::import_skinned_model(path);
        // for (auto &material: data.materials)
        // {
        //     material.load_all();
        // }
        // animations.insert(animations.end(),
        //                      data.animations.begin(), data.animations.end());
        // skinned_models.emplace_back(data);
    }

    std::vector<Model> &get_models()
    {
        return models;
    }

    std::vector<Mesh> &get_meshes()
    {
        return meshes;
    }

    std::vector<SkinnedModel> &get_skinned_models()
    {
        return skinned_models;
    }

    std::vector<Material> &get_materials()
    {
        return materials;
    }

    Material *get_material_by_index(int32_t idx)
    {
        return &materials[idx];
    }

    Mesh *get_mesh_by_name(const std::string &name)
    {
        return &meshes[get_mesh_index_by_name(name)];
    }

    Mesh *get_mesh_by_index(int32_t idx)
    {
        return &meshes[idx];
    }

    int32_t get_mesh_index_by_name(const std::string &name)
    {
        return mesh_index_map[name];
    }

    SkinnedMesh *get_skinned_mesh_by_name(const std::string &name)
    {
        return &skinned_meshes[skinned_mesh_index_map[name]];
    }

    SkinnedMesh *get_skinned_mesh_by_index(int32_t idx)
    {
        return &skinned_meshes[idx];
    }

    int32_t get_skinned_mesh_index_by_name(const std::string &name)
    {
        return skinned_mesh_index_map[name];
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

    void file_added(const std::filesystem::path &path)
    {
        if (path.has_extension() && path.extension().string() == ".glb")
        {
            if (path.parent_path().filename().string() == "models")
            {
                load_model(path.string());
            }
            else if (path.parent_path().filename().string() == "skinned_models")
            {
                load_skinned_model(path.string());
            }
        }
    }

    void print_animations()
    {
        for (auto &anim: animations)
        {
            LOG_INFO("ANIMATION %s", anim.get_name().c_str());
        }
    }

    std::vector<AnimationClip> &get_animations()
    {
        return animations;
    }


    Model *get_model_by_name(const std::string &name)
    {
        auto idx = get_model_index_by_name(name);
        if (idx == -1)
        {
            return nullptr;
        }
        return &models.at(get_model_index_by_name(name));
    }

    Model *get_model_by_index(int32_t idx)
    {
        if (idx > models.size() - 1 || idx < 0)
        {
            return nullptr;
        }
        return &models.at(idx);
    }

    int32_t get_model_index_by_name(const std::string &name)
    {
        return model_index_map[name];
    }

    SkinnedModel *get_skinned_model_by_name(const std::string &name)
    {
        return &skinned_models.at(get_skinned_model_index_by_name(name));
    }

    SkinnedModel *get_skinned_model_by_index(int32_t idx)
    {
        return &skinned_models.at(idx);
    }

    int32_t get_skinned_model_index_by_name(const std::string &name)
    {
        return skinned_model_index_map[name];
    }

    AnimationClip *get_animation_by_name(const std::string &name)
    {
        return &animations.at(animation_index_map[name]);
    }

    AnimationClip *get_animation_by_index(int32_t idx)
    {
        return &animations.at(idx);
    }

    int32_t get_animation_index_by_name(const std::string &name)
    {
        return animation_index_map[name];
    }

    int32_t get_first_animation_index_with_name(const std::string &name)
    {
        for (int32_t i = 0; i < animations.size(); i++)
        {
            std::string anim_name = animations[i].get_name();
            if (anim_name.starts_with(name))
            {
                return i;
            }
        }
        return -1;
    }
}
