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
        header.time_saved = export_time;
        header.aabb_max = model_data.aabb_max;
        header.aabb_min = model_data.aabb_min;
        header.name = model_data.name;
        file.write(reinterpret_cast<const char*>(&header), sizeof(ModelCacheHeader));

        //the mesh datas
        for (const auto& mesh : model_data.meshes)
        {
            MeshCacheHeader mesh_header;
            mesh_header.name = mesh.name;
            mesh_header.aabb_max = mesh.aabb_max;
            mesh_header.aabb_min =mesh.aabb_min;
            mesh_header.index_count = mesh.indices.size();
            mesh_header.vertex_count = mesh.vertices.size();
            mesh_header.material_index = mesh.material_index;
            mesh_header.inverse_bind_pose = mesh.inverse_bind_pose;

            file.write(reinterpret_cast<const char*>(&mesh_header), sizeof(MeshCacheHeader));

            file.write(reinterpret_cast<const char *>(mesh.vertices.data()), mesh.vertices.size() * sizeof(Vertex));
            file.write(reinterpret_cast<const char *>(mesh.indices.data()), mesh.indices.size() * sizeof(uint32_t));
        }
        //the materials
        for (auto& material : model_data.materials)
        {
            std::string path = "cache/textures/" + model_data.name;
            MaterialCacheHeader material_header;
            if (material.albedo.contains_data())
            {
                material_header.albedo_path = path + "/albedo.ctext";
            }
            if (material.roughness.contains_data())
            {
                material_header.roughness_path = path + "/roughness.ctext";
            }
            if (material.ao.contains_data())
            {
                material_header.ao_path = path + "/ao.ctext";
            }
            if (material.emission.contains_data())
            {
                material_header.emission_path = path + "/emission.ctext";
            }
            if (material.metallic.contains_data())
            {
                material_header.metallic_path = path + "/metallic.ctext";
            }
            if (material.normal.contains_data())
            {
                material_header.normal_path = path + "/normal.ctext";
            }
            material_header.metallic_override = material.metallic_override;
            material_header.roughness_override = material.roughness_override;
        }
        file.close();

        for (auto& material : model_data.materials)
        {
            std::string path = "cache/textures/" + model_data.name;
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
        }

        LOG_INFO("Exported model %s!", model_data.name.c_str());
    }
}