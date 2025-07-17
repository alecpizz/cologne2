//
// Created by alecpizz on 7/16/25.
//
#include "AssetManager.h"
#include <filesystem>
#include <fstream>
#include <engine/util/DebugScope.h>
#include <engine/util/FileUtil.h>
#include <algorithm>
#include <string>
#include <vector>
#include <engine/core/Engine.h>
#include <engine/renderer/Renderer.h>

namespace cologne::AssetManager
{
#define NAME_BUFFER_SIZE 256

    void copy_name(char *buffer, const std::string &name, int size = NAME_BUFFER_SIZE)
    {
        std::memset(buffer, 0, size);
        std::memcpy(buffer, name.data(), std::min(static_cast<int>(name.length()), size - 1));
        buffer[std::min(static_cast<int>(name.length()), size - 1)] = '\0';
    }

    void process_material_header(std::ofstream& file, const std::vector<Material>& materials, const std::string& model_name)
    {
        int i = 0;
        for (auto &material: materials)
        {
            std::string path = "cache/textures/" + model_name + "/material" + std::to_string(i);
            MaterialCacheHeader material_header;
            if (material.albedo.contains_data())
            {
                copy_name(material_header.albedo_path, path + "/albedo.ctext", 512);
            }
            if (material.roughness.contains_data())
            {
                copy_name(material_header.roughness_path, path + "/roughness.ctext", 512);
            }
            if (material.ao.contains_data())
            {
                copy_name(material_header.ao_path, path + "/ao.ctext", 512);
            }
            if (material.emission.contains_data())
            {
                copy_name(material_header.emission_path, path + "/emission.ctext", 512);
            }
            if (material.metallic.contains_data())
            {
                copy_name(material_header.metallic_path, path + "/metallic.ctext", 512);
            }
            if (material.normal.contains_data())
            {
                copy_name(material_header.normal_path, path + "/normal.ctext", 512);
            }
            material_header.metallic_override = material.metallic_override;
            material_header.roughness_override = material.roughness_override;
            file.write(reinterpret_cast<const char *>(&material_header), sizeof(MaterialCacheHeader));
            i++;
        }
    }

    void export_material_textures(const std::vector<Material>& materials, const std::string& model_name)
    {
        int i = 0;
        for (auto &material: materials)
        {
            std::string path = "cache/textures/" + model_name + "/material" + std::to_string(i);
            if (material.albedo.contains_data())
            {
                std::string albedo_path = path + "/albedo.ctext";
                material.albedo.export_to_compressed((RESOURCES_PATH + albedo_path).c_str());
            }
            if (material.roughness.contains_data())
            {
                std::string roughness_path = path + "/roughness.ctext";
                material.roughness.export_to_compressed((RESOURCES_PATH + roughness_path).c_str());
            }
            if (material.ao.contains_data())
            {
                std::string ao_path = path + "/ao.ctext";
                material.ao.export_to_compressed((RESOURCES_PATH + ao_path).c_str());
            }
            if (material.emission.contains_data())
            {
                std::string emission_path = path + "/emission.ctext";
                material.emission.export_to_compressed((RESOURCES_PATH + emission_path).c_str());
            }
            if (material.metallic.contains_data())
            {
                std::string metallic_path = path + "/metallic.ctext";
                material.metallic.export_to_compressed((RESOURCES_PATH + metallic_path).c_str());
            }
            if (material.normal.contains_data())
            {
                std::string normal_path = path + "/normal.ctext";
                material.normal.export_to_compressed((RESOURCES_PATH + normal_path).c_str());
            }
            i++;
        }

    }

    void export_model(const ModelData &model_data, uint64_t export_time)
    {
        const std::string output_path = RESOURCES_PATH "cache/models/" + model_data.name + ".cmdl";
        FileUtil::create_directory_recursive(output_path);
        std::ofstream file(output_path, std::ios::binary);
        if (!file.is_open())
        {
            LOG_ERROR("Couldn't open file for writing: %s", output_path);
            return;
        }
        //things to export:
        //the name of the model
        //aabb min
        //aabb max
        ModelCacheHeader header;
        header.mesh_count = model_data.meshes.size();
        header.material_count = model_data.materials.size();
        header.time_saved = export_time;
        header.aabb_max = model_data.aabb_max;
        header.aabb_min = model_data.aabb_min;
        copy_name(header.name, model_data.name);
        file.write(reinterpret_cast<const char *>(&header), sizeof(ModelCacheHeader));

        //the mesh datas
        for (const auto &mesh: model_data.meshes)
        {
            MeshCacheHeader mesh_header;
            copy_name(mesh_header.name, mesh.name);
            mesh_header.aabb_max = mesh.aabb_max;
            mesh_header.aabb_min = mesh.aabb_min;
            mesh_header.index_count = mesh.indices.size();
            mesh_header.vertex_count = mesh.vertices.size();
            mesh_header.material_index = mesh.material_index;
            mesh_header.inverse_bind_pose = mesh.inverse_bind_pose;

            file.write(reinterpret_cast<const char *>(&mesh_header), sizeof(MeshCacheHeader));

            file.write(reinterpret_cast<const char *>(mesh.vertices.data()), mesh.vertices.size() * sizeof(Vertex));
            file.write(reinterpret_cast<const char *>(mesh.indices.data()), mesh.indices.size() * sizeof(uint32_t));
        }
        //the materials
        process_material_header(file, model_data.materials, model_data.name);
        file.close();
        export_material_textures(model_data.materials, model_data.name);
        LOG_INFO("Exported model %s!", model_data.name.c_str());
    }

    ModelData import_model(const char *path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            LOG_ERROR("error importing model at %s", path);
            return {};
        }

        ModelCacheHeader model_header;
        file.read(reinterpret_cast<char *>(&model_header), sizeof(model_header));

        ModelData result_data;
        result_data.name = model_header.name;
        result_data.aabb_max = model_header.aabb_max;
        result_data.aabb_min = model_header.aabb_min;
        result_data.meshes.resize(model_header.mesh_count);
        result_data.materials.resize(model_header.material_count);

        for (uint32_t i = 0; i < model_header.mesh_count; i++)
        {
            MeshData &data = result_data.meshes[i];

            MeshCacheHeader mesh_header;
            file.read(reinterpret_cast<char *>(&mesh_header), sizeof(MeshCacheHeader));

            data.name = mesh_header.name;
            data.aabb_max = mesh_header.aabb_max;
            data.aabb_min = mesh_header.aabb_min;
            data.vertices.resize(mesh_header.vertex_count);
            data.indices.resize(mesh_header.index_count);
            data.material_index = mesh_header.material_index;
            data.inverse_bind_pose = mesh_header.inverse_bind_pose;
            file.read(reinterpret_cast<char *>(data.vertices.data()), mesh_header.vertex_count * sizeof(Vertex));
            file.read(reinterpret_cast<char *>(data.indices.data()), mesh_header.index_count * sizeof(uint32_t));
        }

        for (uint32_t i = 0; i < model_header.material_count; i++)
        {
            Material &material = result_data.materials[i];

            MaterialCacheHeader material_header;
            file.read(reinterpret_cast<char *>(&material_header), sizeof(MaterialCacheHeader));

            material.albedo.set_path(std::string(RESOURCES_PATH) + material_header.albedo_path);
            material.ao.set_path(std::string(RESOURCES_PATH) + material_header.ao_path);
            material.emission.set_path(std::string(RESOURCES_PATH) + material_header.emission_path);
            material.metallic.set_path(std::string(RESOURCES_PATH) + material_header.metallic_path);
            material.roughness.set_path(std::string(RESOURCES_PATH) + material_header.roughness_path);
            material.normal.set_path(std::string(RESOURCES_PATH)  + material_header.normal_path);
            material.roughness_override = material_header.roughness_override;
            material.metallic_override = material_header.metallic_override;
        }
        file.close();

        return result_data;
    }
}
